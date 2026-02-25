#include "BallPlayer.h"
#include "direct3d.h"
#include "key_logger.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include "dust_particle.h"
#include "Goal.h"
#include <ctime>
#include "GoalCollision.h"
#include "pad_logger.h"
#include "player_camera.h"
#include "AirCurveChallenge.h"
#include "UI_GoalAnim.h"
#include "BallPhysics.h"
#include "BallState.h"

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

// ============================================================================
// RENDER
// ============================================================================
static MODEL* g_BallModel = nullptr;

// ============================================================================
// STATE
// ============================================================================
static BallState g_Ball;
static XMFLOAT3 g_StartPosition = { 0,5,0 };

// ============================================================================
// CONSTANTS
// ============================================================================
static constexpr float BALL_RADIUS = 0.3f;
static constexpr float BALL_SCALE = 0.07f;
static constexpr float STOP_SPEED = 0.15f;

// ============================================================================
// DRIBBLE
// ============================================================================
static constexpr float MAX_DRIBBLE_SPEED = 5.0f;
static constexpr float DRIBBLE_ACCEL = 25.0f;
static constexpr float DRIBBLE_DAMPING = 8.0f;

// ============================================================================
// KICK
// ============================================================================
static bool  g_IsKicked = false;
static bool  g_IsChargingKick = false;
static float g_KickCharge = 0.0f;

// Release -> close-up hold -> kick
static bool    g_KickPending = false;
static XMFLOAT3 g_PendingDir = { 0,0,1 };
static float    g_PendingPower = 0.0f;
static float    g_PendingLift = 0.0f;
static float    g_PendingCurve = 0.0f;

static float g_KickDelayTime = 0.0f;
static constexpr float KICK_CINEMATIC_HOLD = 0.3f; // HOLD time (you want ~1.5)

static constexpr float KICK_MIN_POWER = 8.0f;
static constexpr float KICK_MAX_POWER = 45.0f;
static constexpr float KICK_CHARGE_SPEED = 50.0f;

static constexpr float FREEKICK_UP_ANGLE = 0.55f;
static constexpr float FREEKICK_LIFT_EXP = 0.01f;
static constexpr float AIR_GRAVITY_SCALE = 0.85f;

// Air curve
static bool  g_AirCurveUsed = false;
static float g_AirCurveDir = 0.0f;
static float g_AirCurveTime = 0.0f;
static constexpr float AIR_CURVE_DURATION = 0.5f;

static bool  g_HitStopActive = false;
static float g_HitStopTime = 0.0f;
static constexpr float HITSTOP_DURATION = 0.05f;

// ============================================================================
// MATRICES
// ============================================================================
static XMMATRIX g_Rotation = XMMatrixIdentity();
static XMMATRIX g_ModelCorrection = XMMatrixRotationY(XM_PI);
static XMMATRIX g_World = XMMatrixIdentity();

static DustEmitter* g_DustEmitter = nullptr;
static bool g_Scored = false;

// ============================================================================
// KICK
// ============================================================================
void BallPlayer_Kick(const XMFLOAT3& dir, float power, float lift, float curve)
{
    XMVECTOR fwd = XMVector3Normalize(XMVectorSet(dir.x, 0, dir.z, 0));

    float p01 = power / KICK_MAX_POWER;
    float liftBoost = powf(p01, FREEKICK_LIFT_EXP);

    XMVECTOR kickDir =
        XMVector3Normalize(
            fwd * (1.0f - FREEKICK_UP_ANGLE) +
            XMVectorSet(0, FREEKICK_UP_ANGLE, 0, 0)
        );

    g_Ball.velocity.x += XMVectorGetX(kickDir) * power;
    g_Ball.velocity.y += lift * (1.0f + liftBoost * 1.8f);
    g_Ball.velocity.z += XMVectorGetZ(kickDir) * power;

    XMVECTOR side = XMVector3Normalize(
        XMVector3Cross(fwd, XMVectorSet(0, 1, 0, 0))
    );

    float spinScale = power / KICK_MAX_POWER;
    g_Ball.angularVelocity += side * (curve * spinScale);

    g_IsKicked = true;
    g_AirCurveUsed = false;
    g_AirCurveTime = 0.0f;
}

// ============================================================================
// RESET
// ============================================================================
void BallPlayer_Reset()
{
    float z = 1.0f + (rand() / (float)RAND_MAX) * 19.0f;
    Goal_SetWorldOffset({ 0,0,z });

    GoalCollision_ClearBackNetHit();

    BallPhysics_Initialize(g_Ball, g_StartPosition);

    g_IsKicked = false;
    g_IsChargingKick = false;
    g_KickCharge = 0.0f;

    g_AirCurveUsed = false;
    g_AirCurveTime = 0.0f;

    g_Rotation = XMMatrixIdentity();

    g_HitStopActive = false;
    g_HitStopTime = 0.0f;

    // cancel cinematic kick
    g_KickPending = false;
    g_KickDelayTime = 0.0f;

    // IMPORTANT: reset camera lock state
    PlayerCamera_ResetKickCinematic();
}

// ============================================================================
// INIT
// ============================================================================
void BallPlayer_Initialize(const XMFLOAT3& startPos, float)
{
    srand((unsigned)time(NULL));

    g_StartPosition = startPos;
    g_StartPosition.y = 5.0f;

    BallPhysics_Initialize(g_Ball, g_StartPosition);

    g_IsKicked = false;
    g_IsChargingKick = false;
    g_KickCharge = 0.0f;

    g_Rotation = XMMatrixIdentity();

    g_BallModel = ModelLoad("Resources/Model/ball.fbx", 1.0f);
    g_DustEmitter = new DustEmitter(4000, XMVectorZero(), 160.0);
}

// ============================================================================
// UPDATE
// ============================================================================
void BallPlayer_Update(double elapsedTime)
{
    if (KeyLogger_IsTrigger(KK_ENTER) ||
        PadLogger_IsTrigger(0, XINPUT_GAMEPAD_B))
    {
        BallPlayer_Reset();
        g_Scored = false;
        return;
    }

    float dt = (float)elapsedTime;

    // ------------------------------------------------------------
    // KICK CINEMATIC: camera close-up hold, freeze physics, then kick
    // ------------------------------------------------------------
    if (g_KickPending)
    {
        // stop charging state
        g_IsChargingKick = false;

        // lock camera to close-up during hold
        PlayerCamera_BeginKickCinematic();

        // countdown
        g_KickDelayTime -= dt;

        // keep world updated so camera follow feels stable
        g_World =
            XMMatrixScaling(BALL_SCALE, BALL_SCALE, BALL_SCALE) *
            g_Rotation *
            g_ModelCorrection *
            XMMatrixTranslation(
                g_Ball.position.x,
                g_Ball.position.y,
                g_Ball.position.z
            );

        if (g_DustEmitter)
            g_DustEmitter->Update(dt);

        // when timer ends -> kick happens NOW
        if (g_KickDelayTime <= 0.0f)
        {
            // end cinematic (camera will return to normal logic after kick)
            PlayerCamera_EndKickCinematic();

            BallPlayer_Kick(g_PendingDir, g_PendingPower, g_PendingLift, g_PendingCurve);

            g_KickPending = false;
        }

        return; // IMPORTANT: freeze gameplay / no BallPhysics_Update while holding
    }

    bool hasInput =
        KeyLogger_IsPressed(KK_W) ||
        KeyLogger_IsPressed(KK_A) ||
        KeyLogger_IsPressed(KK_S) ||
        KeyLogger_IsPressed(KK_D);

    // DRIBBLE
    if (!g_IsKicked && !g_HitStopActive)
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

        XMVECTOR velXZ = XMVectorSet(g_Ball.velocity.x, 0, g_Ball.velocity.z, 0);
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

        g_Ball.velocity.x = XMVectorGetX(velXZ);
        g_Ball.velocity.z = XMVectorGetZ(velXZ);
    }

    // KICK CHARGE
    if (!g_IsKicked && !g_HitStopActive)
    {
        bool keyboardCharging = KeyLogger_IsPressed(KK_SPACE);
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
            // --------------------------------------------------------
            // RELEASE: SNAP TO CLOSE-UP + HOLD, DO NOT KICK YET
            // --------------------------------------------------------
            float power = std::max(g_KickCharge, KICK_MIN_POWER);

            XMFLOAT3 camFront = PlayerCamera_GetFront();
            XMVECTOR dirVec = XMVectorSet(camFront.x, 0.0f, camFront.z, 0.0f);
            dirVec = XMVector3Normalize(dirVec);

            XMFLOAT3 shootDir;
            XMStoreFloat3(&shootDir, dirVec);

            // store kick for later
            g_PendingDir = shootDir;
            g_PendingPower = power;
            g_PendingLift = power * 0.18f;
            g_PendingCurve = power * 0.06f;

            // start cinematic hold
            g_KickPending = true;
            g_KickDelayTime = KICK_CINEMATIC_HOLD;

            // clear charging state
            g_IsChargingKick = false;
            g_KickCharge = 0.0f;

            // also: force camera to snap close immediately on release
            PlayerCamera_SnapCloseNow();

            return;
        }
    }

    // normal physics (only when not pending)
    BallPhysics_Update(g_Ball, dt);

    // GOAL COLLISION
    Goal_HandleBallCollision(g_Ball.position, g_Ball.velocity, BALL_RADIUS);

    if (!g_Scored && GoalCollision_BackNetTouched())
    {
        g_Scored = true;
        UI_GoalAnim_Play();
    }

    // STOP
    XMVECTOR velXZ = XMVectorSet(g_Ball.velocity.x, 0, g_Ball.velocity.z, 0);
    float speed = XMVectorGetX(XMVector3Length(velXZ));

    if (g_Ball.onGround && speed < STOP_SPEED)
    {
        if (!hasInput)
        {
            g_Ball.velocity = { 0,0,0 };
            g_Ball.angularVelocity = XMVectorZero();
        }
    }

    // VISUAL ROLL
    if (speed > 0.05f)
    {
        XMVECTOR axis = XMVector3Normalize(
            XMVector3Cross(XMVector3Normalize(velXZ), XMVectorSet(0, 1, 0, 0))
        );

        g_Rotation *= XMMatrixRotationAxis(
            axis,
            speed * dt / BALL_RADIUS
        );
    }

    // WORLD
    g_World =
        XMMatrixScaling(BALL_SCALE, BALL_SCALE, BALL_SCALE) *
        g_Rotation *
        g_ModelCorrection *
        XMMatrixTranslation(
            g_Ball.position.x,
            g_Ball.position.y,
            g_Ball.position.z
        );

    if (g_DustEmitter)
        g_DustEmitter->Update(dt);
}

void BallPlayer_Draw()
{
    if (g_BallModel)
        ModelDraw(g_BallModel, g_World);

    if (g_DustEmitter)
        g_DustEmitter->Render();
}

void BallPlayer_Finalize()
{
    delete g_DustEmitter;
    g_DustEmitter = nullptr;

    if (g_BallModel) ModelRelease(g_BallModel);
    g_BallModel = nullptr;
}

XMFLOAT3 BallPlayer_GetPosition()
{
    return g_Ball.position;
}

void BallPlayer_SetPosition(const XMFLOAT3& pos)
{
    g_Ball.position = pos;
}

XMMATRIX BallPlayer_GetWorldMatrix()
{
    return g_World;
}

MODEL* BallPlayer_GetModel()
{
    return g_BallModel;
}

float BallPlayer_GetKickCharge()
{
    return g_KickCharge;
}

bool BallPlayer_IsCharging()
{
    return g_IsChargingKick;
}

float BallPlayer_GetKickMaxPower()
{
    return KICK_MAX_POWER;
}

bool BallPlayer_IsKicked()
{
    return g_IsKicked;
}