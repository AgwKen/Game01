#include "BallControl.h"

#include "key_logger.h"
#include "pad_logger.h"
#include "player_camera.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

// ------------------------------------------------------------
// MANUAL CLAMP (no std::clamp)
// ------------------------------------------------------------
static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float ClampFloat(float v, float mn, float mx)
{
    if (v < mn) return mn;
    if (v > mx) return mx;
    return v;
}

// ============================================================================
// CONSTANTS (moved here because this module controls inputs)
// ============================================================================

// DRIBBLE
static constexpr float MAX_DRIBBLE_SPEED = 5.0f;
static constexpr float DRIBBLE_ACCEL = 25.0f;
static constexpr float DRIBBLE_DAMPING = 8.0f;

// KICK
static bool  g_IsChargingKick = false;
static float g_KickCharge = 0.0f;

static constexpr float KICK_MIN_POWER = 8.0f;
static constexpr float KICK_MAX_POWER = 45.0f;
static constexpr float KICK_CHARGE_SPEED = 30.0f;

static constexpr float KICK_CINEMATIC_MIN_CHARGE01 = 0.6f;

// AIM INPUT (Arrow keys / Right Stick)
static XMFLOAT2 g_AimInput = { 0.0f, 0.0f };   // x = left/right, y = forward/back
static float    g_AimStrength = 0.55f;         // how much arrow/right stick bends aim

// AIR CURVE CONTROL WINDOW (PLAYER CONTROL AFTER RELEASE)
static float g_AirCurveControlTimer = 0.0f;
static constexpr float AIR_CURVE_CONTROL_DURATION = 1.5f; // <-- 1.5 seconds

// How strong player control changes spin during air
static constexpr float AIR_CURVE_CONTROL_SPIN_STRENGTH = 50.0f;
static constexpr float AIR_CURVE_Y_DECAY = 16.0f; // bigger = curve stops faster

// ------------------------------------------------------------
// AIM CAMERA LOCK (for controller right-stick aiming)
// ------------------------------------------------------------
static bool g_AimCameraLocked = false;
static XMFLOAT3 g_AimCameraForward = { 0.0f, 0.0f, 1.0f };

static constexpr float AIM_STICK_DEADZONE = 0.15f;
static constexpr float AIM_CAMERA_TURN_SPEED = 22.0f;
static constexpr float AIM_LOOK_AHEAD = 2.0f;

static XMFLOAT3 NormalizeXZ(const XMFLOAT3& v)
{
    float lenSq = v.x * v.x + v.z * v.z;
    if (lenSq < 0.0001f)
        return { 0.0f, 0.0f, 1.0f };

    float invLen = 1.0f / sqrtf(lenSq);
    return { v.x * invLen, 0.0f, v.z * invLen };
}


// ============================================================================
// AIM HELPER (use camera forward + aim input)
// ============================================================================
XMFLOAT3 BallControl_BuildAimDirection()
{
    // Base direction = camera forward (XZ only)
    XMFLOAT3 camFront = PlayerCamera_GetFront();
    XMVECTOR base = XMVectorSet(camFront.x, 0.0f, camFront.z, 0.0f);

    if (XMVectorGetX(XMVector3LengthSq(base)) < 0.0001f)
        base = XMVectorSet(0, 0, 1, 0);

    base = XMVector3Normalize(base);

    // Right vector from base
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, base));

    // Apply aim input
    // g_AimInput.x -> right/left, g_AimInput.y -> forward/back
    XMVECTOR aim =
        base + right * (g_AimInput.x * g_AimStrength) + base * (g_AimInput.y * 0.45f);

    if (XMVectorGetX(XMVector3LengthSq(aim)) < 0.0001f)
        aim = base;

    aim = XMVector3Normalize(aim);

    XMFLOAT3 out;
    XMStoreFloat3(&out, aim);
    return out;
}

XMFLOAT3 BallControl_GetAimDirection(bool isKicked)
{
    // If kicked, aim doesn't matter
    if (isKicked) return { 0,0,1 };
    return BallControl_BuildAimDirection();
}

// ============================================================================
// PUBLIC API
// ============================================================================
void BallControl_Reset()
{
    g_IsChargingKick = false;
    g_KickCharge = 0.0f;

    g_AimInput = { 0.0f, 0.0f };

    g_AirCurveControlTimer = 0.0f;
}

void BallControl_UpdateAimInput()
{
    // Arrow keys (keyboard)
    float ax = 0.0f, ay = 0.0f;
    if (KeyLogger_IsPressed(KK_LEFT))  ax -= 1.0f;
    if (KeyLogger_IsPressed(KK_RIGHT)) ax += 1.0f;
    if (KeyLogger_IsPressed(KK_UP))    ay += 1.0f;
    if (KeyLogger_IsPressed(KK_DOWN))  ay -= 1.0f;

    // Right stick (controller)
    XMFLOAT2 rightStick = PadLogger_GetRightThumbStick(0);

    // Prefer stick if used
    if (fabs(rightStick.x) > 0.15f || fabs(rightStick.y) > 0.15f)
    {
        auto Curve = [](float v)
            {
                float s = (v < 0.0f) ? -1.0f : 1.0f;
                float a = fabs(v);

                // curve: small inputs become much smaller (better precision)
                a = powf(a, 1.8f);

                return s * a;
            };

        g_AimInput.x = ClampFloat(Curve(rightStick.x), -1.0f, 1.0f);
        g_AimInput.y = ClampFloat(Curve(rightStick.y), -1.0f, 1.0f);
    }
    else
    {
        g_AimInput.x = ax;
        g_AimInput.y = ay;
    }

    // Small deadzone
    if (fabs(g_AimInput.x) < 0.10f) g_AimInput.x = 0.0f;
    if (fabs(g_AimInput.y) < 0.10f) g_AimInput.y = 0.0f;
}
void BallControl_UpdateDribble(float dt, BallState& ball, bool isKicked, bool hitStopActive)
{
    // DRIBBLE (exact code moved)
    if (!isKicked && !hitStopActive)
    {
        XMFLOAT3 in = { 0,0,0 };
        XMFLOAT2 stick = PadLogger_GetLeftThumbStick(0);

        if (fabs(stick.x) > 0.05f || fabs(stick.y) > 0.05f)
        {
            in.x = stick.x;
            in.z = stick.y;
        }
        else
        {
            if (KeyLogger_IsPressed(KK_W)) in.z += 1;
            if (KeyLogger_IsPressed(KK_S)) in.z -= 1;
            if (KeyLogger_IsPressed(KK_A)) in.x -= 1;
            if (KeyLogger_IsPressed(KK_D)) in.x += 1;
        }

        XMVECTOR velXZ = XMVectorSet(ball.velocity.x, 0, ball.velocity.z, 0);
        XMVECTOR dir = XMVectorSet(in.x, 0, in.z, 0);

        if (XMVectorGetX(XMVector3LengthSq(dir)) > 0.001f)
        {
            dir = XMVector3Normalize(dir);
            velXZ += dir * DRIBBLE_ACCEL * dt;
        }
        else
        {
            float damp = std::exp(-DRIBBLE_DAMPING * dt);
            velXZ *= damp;
        }

        float speed = XMVectorGetX(XMVector3Length(velXZ));
        if (speed > MAX_DRIBBLE_SPEED)
            velXZ = XMVector3Normalize(velXZ) * MAX_DRIBBLE_SPEED;

        if (speed < 0.05f)
            velXZ = XMVectorZero();

        ball.velocity.x = XMVectorGetX(velXZ);
        ball.velocity.z = XMVectorGetZ(velXZ);
    }
}

BallKickRelease BallControl_UpdateKickCharge(float dt, bool isKicked, bool hitStopActive)
{
    BallKickRelease out{};

    // KICK CHARGE (exact logic moved)
    if (!isKicked && !hitStopActive)
    {
        // TEMP: disable keyboard kick
        bool keyboardCharging = false;

        float triggerValue = PadLogger_GetRightTrigger(0);
        bool controllerCharging = triggerValue > 0.1f;

        if (keyboardCharging || controllerCharging)
        {
            g_IsChargingKick = true;

            float chargeAmount = 0.0f;
            if (keyboardCharging)
                chargeAmount = KICK_CHARGE_SPEED * dt;
            if (controllerCharging)
                chargeAmount = triggerValue * KICK_CHARGE_SPEED * dt * 2.0f;

            g_KickCharge += chargeAmount;
            g_KickCharge = std::min(g_KickCharge, KICK_MAX_POWER);
        }
        else if (g_IsChargingKick)
        {
            // RELEASE
            float power = std::max(g_KickCharge, KICK_MIN_POWER);

            float charge01 = g_KickCharge / KICK_MAX_POWER;
            charge01 = Clamp01(charge01);

            // NEW: Aim direction is camera + arrow/right-stick aim
            XMFLOAT3 shootDir = BallControl_BuildAimDirection();

            const bool doCinematic = (charge01 >= KICK_CINEMATIC_MIN_CHARGE01);

            out.released = true;
            out.doCinematic = doCinematic;

            out.dir = shootDir;
            out.power = power;
            out.lift = power * 0.18f;

            // NOTE: initial curve amount (pre-spin). Keep this small.
            // Real curve now comes from BallPhysics Magnus + 1.5 sec control.
            out.curve = power * 0.06f * g_AimInput.x;  // left/right curve

            g_IsChargingKick = false;
            g_KickCharge = 0.0f;
        }
    }

    return out;
}

// ============================================================================
// AIR CURVE CONTROL (WASD / Left Stick) for 1.5 seconds after kick
// This does NOT teleport the ball ? it adds spin, and BallPhysics uses spin to curve.
// ============================================================================
void BallControl_UpdateAirCurveControl(float dt, BallState& ball, bool isKicked)
{
    if (!isKicked) return;
    if (ball.onGround) return;

    if (g_AirCurveControlTimer >= AIR_CURVE_CONTROL_DURATION)
    {
        // After window ends, kill curve spin fast
        float damp = std::exp(-AIR_CURVE_Y_DECAY * dt);

        XMFLOAT3 w;
        XMStoreFloat3(&w, ball.angularVelocity);
        w.y *= damp;
        ball.angularVelocity = XMVectorSet(w.x, w.y, w.z, 0.0f);
        return;
    }

    g_AirCurveControlTimer += dt;

    // Keyboard curve (A/D)
    float keyX = 0.0f;
    if (KeyLogger_IsPressed(KK_A)) keyX -= 1.0f;
    if (KeyLogger_IsPressed(KK_D)) keyX += 1.0f;

    // Optional: allow W/S to influence "topspin/backspin" feel (comment out if unwanted)
    float keyY = 0.0f;
    if (KeyLogger_IsPressed(KK_W)) keyY += 1.0f;
    if (KeyLogger_IsPressed(KK_S)) keyY -= 1.0f;

    // Controller left stick curve
    XMFLOAT2 left = PadLogger_GetLeftThumbStick(0);
    float stickX = left.x;   // left/right curve
    float stickY = left.y;   // optional up/down spin

    float inputX = keyX;
    float inputY = keyY;

    // If stick being used, prefer it
    if (fabs(stickX) > 0.15f) inputX = stickX;
    if (fabs(stickY) > 0.15f) inputY = stickY;

    // Deadzone
    if (fabs(inputX) < 0.10f) inputX = 0.0f;
    if (fabs(inputY) < 0.10f) inputY = 0.0f;

    if (inputX == 0.0f && inputY == 0.0f)
    {
        // When player releases, quickly stop curve by damping ONLY Y spin
        float damp = std::exp(-AIR_CURVE_Y_DECAY * dt);

        XMFLOAT3 w;
        XMStoreFloat3(&w, ball.angularVelocity);
        w.y *= damp; // kill curve spin
        ball.angularVelocity = XMVectorSet(w.x, w.y, w.z, 0.0f);
        return;
    }

    // Determine current travel direction (XZ)
    XMVECTOR v = XMVectorSet(ball.velocity.x, 0.0f, ball.velocity.z, 0.0f);
    if (XMVectorGetX(XMVector3LengthSq(v)) < 0.0001f)
        return;

    XMVECTOR fwd = XMVector3Normalize(v);
    XMVECTOR upAxis = XMVectorSet(0, 1, 0, 0);

    // Right axis relative to travel direction (for optional topspin/backspin)
    XMVECTOR rightAxis = XMVector3Normalize(XMVector3Cross(upAxis, fwd));

    float spinPush = AIR_CURVE_CONTROL_SPIN_STRENGTH;

    // A/D = LEFT/RIGHT CURVE (spin around UP)
    ball.angularVelocity += upAxis * (inputX * spinPush * dt);

    // W/S = optional DIP/LIFT feel (spin around RIGHT)
    ball.angularVelocity += rightAxis * (inputY * (spinPush * 0.6f) * dt);
}

void BallControl_OnKickApplied()
{
    // Start curve control window on release (same behavior as before)
    g_AirCurveControlTimer = 0.0f;
}

// Getters
float BallControl_GetKickCharge() { return g_KickCharge; }
bool  BallControl_IsCharging() { return g_IsChargingKick; }
float BallControl_GetKickMaxPower() { return KICK_MAX_POWER; }