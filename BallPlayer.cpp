#include "BallPlayer.h"
#include "direct3d.h"
#include "key_logger.h"
#include "terrain.h"
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

// ============================================================================
// RENDER
// ============================================================================
static MODEL* g_BallModel = nullptr;

// ============================================================================
// STATE
// ============================================================================
static XMFLOAT3 g_Position = { 0,5,0 };
static XMFLOAT3 g_PrevPosition = { 0,5,0 };
static XMFLOAT3 g_Velocity = { 0,0,0 };
static XMFLOAT3 g_StartPosition = { 0,5,0 };

static XMVECTOR g_AngularVelocity = XMVectorZero();

// ============================================================================
// PHYSICS CONSTANTS
// ============================================================================
static constexpr float BALL_RADIUS = 0.3f;
static constexpr float BALL_SCALE  = 0.07f;
static constexpr float GRAVITY     = -25.0f;

// Ground
static constexpr float BASE_FRICTION = 6.0f;
static constexpr float STOP_SPEED    = 0.15f;
static constexpr float STOP_SPIN     = 0.2f;

// Bounce
static constexpr float BOUNCE_RESTITUTION = 0.35f;
static constexpr float SPIN_LOSS_ON_BOUNCE = 0.7f;

// ============================================================================
// DRIBBLE
// ============================================================================
static constexpr float MAX_DRIBBLE_SPEED = 5.0f;
static constexpr float DRIBBLE_ACCEL     = 25.0f;
static constexpr float DRIBBLE_DAMPING   = 8.0f;

// ============================================================================
// KICK
// ============================================================================
static bool  g_IsKicked = false;
static bool  g_IsChargingKick = false;
static float g_KickCharge = 0.0f;

static constexpr float KICK_MIN_POWER = 8.0f;
static constexpr float KICK_MAX_POWER = 35.0f;
static constexpr float KICK_CHARGE_SPEED = 50.0f;

static constexpr float FREEKICK_UP_ANGLE   = 0.55f;
static constexpr float FREEKICK_LIFT_EXP   = 0.01f;
static constexpr float AIR_GRAVITY_SCALE   = 0.85f;

// Air curve
static bool  g_AirCurveUsed = false;
static float g_AirCurveDir  = 0.0f;
static float g_AirCurveTime = 0.0f;
static constexpr float AIR_CURVE_DURATION = 0.5f;

// ============================================================================
// MATRICES
// ============================================================================
static XMMATRIX g_Rotation = XMMatrixIdentity();
static XMMATRIX g_ModelCorrection = XMMatrixRotationY(XM_PI);
static XMMATRIX g_World = XMMatrixIdentity();

static DustEmitter* g_DustEmitter = nullptr;

static bool g_Scored = false;


// ============================================================================
// TERRAIN NORMAL
// ============================================================================
static XMFLOAT3 GetTerrainNormal(float x, float z)
{
    const float eps = 0.5f;

    float hL = Mesh_GetHeightAt(x - eps, z);
    float hR = Mesh_GetHeightAt(x + eps, z);
    float hD = Mesh_GetHeightAt(x, z - eps);
    float hU = Mesh_GetHeightAt(x, z + eps);

    XMFLOAT3 n = { hL - hR, 2.0f, hD - hU };
    XMStoreFloat3(&n, XMVector3Normalize(XMLoadFloat3(&n)));
    return n;
}

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

    g_Velocity.x += XMVectorGetX(kickDir) * power;
    g_Velocity.y += lift * (1.0f + liftBoost * 1.8f);
    g_Velocity.z += XMVectorGetZ(kickDir) * power;

    XMVECTOR side = XMVector3Normalize(
        XMVector3Cross(fwd, XMVectorSet(0, 1, 0, 0))
    );
    float spinScale = power / KICK_MAX_POWER;   // 0 → 1 range
    g_AngularVelocity += side * (curve * spinScale);

    g_IsKicked = true;

    g_AirCurveUsed = false;
    g_AirCurveTime = 0.0f;
}
void BallPlayer_Reset()
{
    float z = 1.0f + (rand() / (float)RAND_MAX) * 19.0f;
    Goal_SetWorldOffset({ 0,0,z });

    GoalCollision_ClearBackNetHit();

    g_Position = g_StartPosition;
    g_PrevPosition = g_StartPosition;

    g_Velocity = { 0,0,0 };
    g_AngularVelocity = XMVectorZero();

    g_IsKicked = false;
    g_IsChargingKick = false;
    g_KickCharge = 0.0f;

    g_AirCurveUsed = false;
    g_AirCurveTime = 0.0f;

    g_Rotation = XMMatrixIdentity();
}

// ============================================================================
// INIT
// ============================================================================
void BallPlayer_Initialize(const XMFLOAT3& startPos, float)
{
    srand((unsigned)time(NULL));

    g_StartPosition = startPos;
    g_StartPosition.y = 5.0f;

    g_Position = g_StartPosition;
    g_PrevPosition = g_Position;


    g_Velocity = { 0,0,0 };
    g_AngularVelocity = XMVectorZero();

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
    // --------------------------------------------------------
// FORCE SKIP / RESET (Keyboard + Controller B)
// --------------------------------------------------------
    if (KeyLogger_IsTrigger(KK_ENTER) ||
        PadLogger_IsTrigger(0, XINPUT_GAMEPAD_B))
    {
        BallPlayer_Reset();
        g_Scored = false;
        return;
    }

    float dt = (float)elapsedTime;

    bool hasInput =
        KeyLogger_IsPressed(KK_W) ||
        KeyLogger_IsPressed(KK_A) ||
        KeyLogger_IsPressed(KK_S) ||
        KeyLogger_IsPressed(KK_D);

    // ------------------------------------------------------------------------
    // DRIBBLE (GROUND CONTROL)
    // ------------------------------------------------------------------------
    if (!g_IsKicked)
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
            // ⌨ Keyboard fallback
            if (KeyLogger_IsPressed(KK_W)) in.z += 1;
            if (KeyLogger_IsPressed(KK_S)) in.z -= 1;
            if (KeyLogger_IsPressed(KK_A)) in.x -= 1;
            if (KeyLogger_IsPressed(KK_D)) in.x += 1;
        }


        XMVECTOR velXZ = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
        XMVECTOR dir   = XMVectorSet(in.x, 0, in.z, 0);

        float speed = XMVectorGetX(XMVector3Length(velXZ));

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

        speed = XMVectorGetX(XMVector3Length(velXZ));
        if (speed > MAX_DRIBBLE_SPEED)
            velXZ = XMVector3Normalize(velXZ) * MAX_DRIBBLE_SPEED;

        if (speed < 0.05f)
            velXZ = XMVectorZero();

        g_Velocity.x = XMVectorGetX(velXZ);
        g_Velocity.z = XMVectorGetZ(velXZ);
    }

    // ------------------------------------------------------------------------
    // KICK CHARGE (KEYBOARD + CONTROLLER HYBRID)
    // ------------------------------------------------------------------------
    if (!g_IsKicked)
    {
        bool keyboardCharging = KeyLogger_IsPressed(KK_SPACE);
        float triggerValue = PadLogger_GetRightTrigger(0);
        bool controllerCharging = triggerValue > 0.1f;

        // If either input is charging
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
        // Release when BOTH are not charging
        else if (g_IsChargingKick)
        {
            float power = std::max(g_KickCharge, KICK_MIN_POWER);

            // ============================
            // DIRECTION FROM CAMERA
            // ============================

            XMFLOAT3 camFront = PlayerCamera_GetFront();

            XMVECTOR dirVec = XMVectorSet(camFront.x, 0.0f, camFront.z, 0.0f);
            dirVec = XMVector3Normalize(dirVec);

            XMFLOAT3 shootDir;
            XMStoreFloat3(&shootDir, dirVec);

            BallPlayer_Kick(
                shootDir,
                power,
                power * 0.18f,
                power * 0.06f
            );

            g_IsChargingKick = false;
            g_KickCharge = 0.0f;
        }
    }

    // ------------------------------------------------------------------------
    // GRAVITY
    // ------------------------------------------------------------------------
    float gravityScale = (g_Velocity.y > 0.0f && g_IsKicked) ? AIR_GRAVITY_SCALE : 1.0f;
    g_Velocity.y += GRAVITY * gravityScale * dt;

    // ------------------------------------------------------------------------
    // MOVE
    // ------------------------------------------------------------------------
    g_PrevPosition = g_Position;
    g_Position.x += g_Velocity.x * dt;
    g_Position.y += g_Velocity.y * dt;
    g_Position.z += g_Velocity.z * dt;
    // ---- GOAL PHYSICAL COLLISION ----


    // ------------------------------------------------------------------------
    // TERRAIN COLLISION
    // ------------------------------------------------------------------------
    float ground = Mesh_GetHeightAt(g_Position.x, g_Position.z);
    bool onGround = false;

    if (g_Position.y - BALL_RADIUS < ground)
    {
        g_Position.y = ground + BALL_RADIUS;

        XMFLOAT3 n = GetTerrainNormal(g_Position.x, g_Position.z);
        XMVECTOR normal = XMLoadFloat3(&n);
        XMVECTOR vel = XMLoadFloat3(&g_Velocity);

        float vn = XMVectorGetX(XMVector3Dot(vel, normal));
        if (vn < 0.0f)
        {
            vel -= normal * (1.0f + BOUNCE_RESTITUTION) * vn;
            g_AngularVelocity *= SPIN_LOSS_ON_BOUNCE;
        }

        XMStoreFloat3(&g_Velocity, vel);
        if (n.y > 0.4f) onGround = true;

        // ----- DUST EFFECT LOGIC -----
        if (onGround)
        {
            float moveSpeed = sqrtf(
                g_Velocity.x * g_Velocity.x +
                g_Velocity.z * g_Velocity.z
            );

            if (moveSpeed > 1.2f)
            {
                g_DustEmitter->SetPosition(
                    DirectX::XMVectorSet(
                        g_Position.x,
                        ground + 0.02f,
                        g_Position.z,
                        0));

                g_DustEmitter->Emmit(true);
            }
            else
            {
                g_DustEmitter->Emmit(false);
            }
        }
        else
        {
            g_DustEmitter->Emmit(false);
        }

    }
    Goal_HandleBallCollision(g_Position, g_Velocity, BALL_RADIUS);
    if (!g_Scored && GoalCollision_BackNetTouched())
    {
        g_Scored = true;
        UI_GoalAnim_Play();
    }
    // ------------------------------------------------------------------------
    // MAGNUS (ONLY WHEN KICKED)
    // ------------------------------------------------------------------------
    if (g_IsKicked)
    {
        XMVECTOR velXZ = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
        if (XMVectorGetX(XMVector3Length(g_AngularVelocity)) > 0.01f)
        {
            XMVECTOR mag = XMVector3Cross(g_AngularVelocity, velXZ)  * 0.015f;
            g_Velocity.x += XMVectorGetX(mag);
            g_Velocity.z += XMVectorGetZ(mag);
            g_AngularVelocity *= 0.99f;
        }
    }
    // ------------------------------------------------------------------------
// GROUND FRICTION (AFTER KICK)
// ------------------------------------------------------------------------
    if (onGround && g_IsKicked)
    {
        XMVECTOR velXZ = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
        float speed = XMVectorGetX(XMVector3Length(velXZ));

        if (speed > 0.0f)
        {
            // light rolling friction (tuned for slopes)
            float friction = 3.5f;
            speed = std::max(0.0f, speed - friction * dt);

            if (speed > 0.01f)
            {
                velXZ = XMVector3Normalize(velXZ) * speed;
                g_Velocity.x = XMVectorGetX(velXZ);
                g_Velocity.z = XMVectorGetZ(velXZ);
            }
            else
            {
                g_Velocity.x = 0.0f;
                g_Velocity.z = 0.0f;
            }
        }
    }


    // ------------------------------------------------------------------------
    // STOP
    // ------------------------------------------------------------------------
    XMVECTOR velXZ = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
    float speed = XMVectorGetX(XMVector3Length(velXZ));

    if (onGround && speed < STOP_SPEED)
    {
        if (g_Scored)
        {
            static float timer = 0;
            timer += dt;

            if (timer > 1.5f)
            {
                BallPlayer_Reset();
                g_Scored = false;
                timer = 0;
            }
        }
        else
        {
            if (g_IsKicked)
            {
                BallPlayer_Reset();
                return;
            }
        }

        if (!hasInput)
        {
            g_Velocity = { 0,0,0 };
            g_AngularVelocity = XMVectorZero();
        }
    }


    // ------------------------------------------------------------------------
    // VISUAL ROLL
    // ------------------------------------------------------------------------
    if (speed > 0.05f)
    {
        XMVECTOR axis = XMVector3Normalize(
            XMVector3Cross(XMVector3Normalize(velXZ), XMVectorSet(0,1,0,0))
        );

        g_Rotation *= XMMatrixRotationAxis(
            axis,
            speed * dt / BALL_RADIUS
        );
    }

    // ------------------------------------------------------------------------
    // AIR CURVE (ONE-TIME HYBRID)
    // ------------------------------------------------------------------------
    if (!onGround && g_IsKicked)
    {
        if (!g_AirCurveUsed)
        {
            float curveDir = 0.0f;

            // Keyboard arrows for curve
            if (KeyLogger_IsPressed(KK_LEFT))  curveDir -= 1.0f;
            if (KeyLogger_IsPressed(KK_RIGHT)) curveDir += 1.0f;

            XMFLOAT2 leftStick = PadLogger_GetLeftThumbStick(0);
            curveDir += leftStick.x;


            if (fabs(curveDir) > 0.2f) // threshold so tiny stick doesn't trigger
            {
                g_AirCurveUsed = true;
                g_AirCurveDir = (curveDir > 0.0f) ? 1.0f : -1.0f;
                g_AirCurveTime = 0.0f;
            }
        }

        if (g_AirCurveUsed && g_AirCurveTime < AIR_CURVE_DURATION)
        {
            g_AirCurveTime += dt;

            XMVECTOR vel = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
            float speed = XMVectorGetX(XMVector3Length(vel));

            if (speed > 0.01f)
            {
                float curveStrength = 0.6f;

                // Get horizontal direction
                XMVECTOR dir = XMVector3Normalize(vel);

                // Side direction relative to movement
                XMVECTOR up = XMVectorSet(0, 1, 0, 0);
                XMVECTOR side = XMVector3Cross(up, dir);  // perpendicular

                float curveAmount = g_AirCurveDir * curveStrength * dt;

                // Add sideways force relative to shot direction
                vel += side * curveAmount * speed;

                // Keep original speed
                vel = XMVector3Normalize(vel) * speed;


                g_Velocity.x = XMVectorGetX(vel);
                g_Velocity.z = XMVectorGetZ(vel);
            }
        }

    }

    // ------------------------------------------------------------------------
    // WORLD
    // ------------------------------------------------------------------------
    g_World =
        XMMatrixScaling(BALL_SCALE, BALL_SCALE, BALL_SCALE) *
        g_Rotation *
        g_ModelCorrection *
        XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);

    // update dust particles
    if (g_DustEmitter)
        g_DustEmitter->Update(dt);

}

// ============================================================================
// DRAW / CLEANUP
// ============================================================================
void BallPlayer_Draw()
{
    if (g_BallModel)
        ModelDraw(g_BallModel, g_World);

    // draw dust after ball
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

XMFLOAT3 BallPlayer_GetPosition() { return g_Position; }
void BallPlayer_SetPosition(const XMFLOAT3& pos)
{
    g_Position = pos;
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