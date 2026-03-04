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
#include "game.h"
#include "RunManager.h"

// NEW:
#include "BallControl.h"
#include "trajetory3d.h"

using namespace DirectX;

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
// KICK
// ============================================================================
static bool  g_IsKicked = false;

// Release -> close-up hold -> kick
static bool     g_KickPending = false;
static XMFLOAT3 g_PendingDir = { 0,0,1 };
static float    g_PendingPower = 0.0f;
static float    g_PendingLift = 0.0f;
static float    g_PendingCurve = 0.0f;

static float g_KickDelayTime = 0.0f;
static constexpr float KICK_CINEMATIC_HOLD = 0.3f;

static constexpr float FREEKICK_UP_ANGLE = 0.55f;
static constexpr float FREEKICK_LIFT_EXP = 0.01f;
static constexpr float AIR_GRAVITY_SCALE = 0.85f;

// ============================================================================
// Air curve flags
// ============================================================================
static bool  g_AirCurveUsed = false;
static float g_AirCurveDir = 0.0f;
static float g_AirCurveTime = 0.0f;
static constexpr float AIR_CURVE_DURATION = 1.5f;

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
static XMFLOAT3 g_PrevBallPos = { 0,0,0 };

// ============================================================================
// CURVE TRAIL (trajectory)
// ============================================================================
static float g_TrajSpawnAcc = 0.0f;

// ============================================================================
// KICK
// ============================================================================
void BallPlayer_Kick(const XMFLOAT3& dir, float power, float lift, float curve)
{
    float KICK_MAX_POWER = BallControl_GetKickMaxPower();

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

    // LEFT/RIGHT CURVE = spin around WORLD UP (Y axis)
    XMVECTOR upAxis = XMVectorSet(0, 1, 0, 0);
    float spinScale = power / KICK_MAX_POWER;

    // If curve direction feels reversed, change to: -(curve * spinScale)
    g_Ball.angularVelocity += upAxis * (curve * spinScale);

    g_IsKicked = true;

    BallControl_OnKickApplied();

    g_AirCurveUsed = false;
    g_AirCurveTime = 0.0f;

    // reset curve trail spawn timer so it starts clean on first curve
    g_TrajSpawnAcc = 0.0f;
}

// ============================================================================
// RESET
// ============================================================================
void BallPlayer_Reset()
{
    g_Scored = false;
    float x = (float)((rand() % 10) - 5);
    float z = 1.0f + (rand() / (float)RAND_MAX) * 19.0f;
    Goal_SetWorldOffset({ x,0,z });

    GoalCollision_ClearBackNetHit();

    BallPhysics_Initialize(g_Ball, g_StartPosition);

    g_IsKicked = false;

    BallControl_Reset();

    g_AirCurveUsed = false;
    g_AirCurveTime = 0.0f;

    g_Rotation = XMMatrixIdentity();

    g_HitStopActive = false;
    g_HitStopTime = 0.0f;

    g_KickPending = false;
    g_KickDelayTime = 0.0f;

    g_TrajSpawnAcc = 0.0f;

    PlayerCamera_ResetKickCinematic();
    Game_OnGoalReset();
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

    BallControl_Reset();

    g_Rotation = XMMatrixIdentity();

    g_BallModel = ModelLoad("Resources/Model/ball.fbx", 1.0f);
    g_DustEmitter = new DustEmitter(4000, XMVectorZero(), 160.0);

    // Trajectory effect system
    Trajetory3d_Initialize();
    g_TrajSpawnAcc = 0.0f;
}

// ============================================================================
// UPDATE
// ============================================================================
void BallPlayer_Update(double elapsedTime)
{
    if (Run_IsActive())
    {
        if (KeyLogger_IsTrigger(KK_ENTER) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_B))
        {
            BallPlayer_Reset();
            g_Scored = false;
            return;
        }
    }

    float dt = (float)elapsedTime;

    // update trajectory lifetime
    Trajetory3d_Update(dt);

    // ------------------------------------------------------------
    // AIM INPUT UPDATE (Arrow keys + Right Stick)
    // Only meaningful while not kicked (before shot)
    // ------------------------------------------------------------
    if (!g_IsKicked)
    {
        BallControl_UpdateAimInput();
    }

    // ------------------------------------------------------------
    // KICK CINEMATIC: close-up hold, freeze physics, then kick
    // ------------------------------------------------------------
    if (g_KickPending)
    {
        PlayerCamera_BeginKickCinematic();

        g_KickDelayTime -= dt;

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

        if (g_KickDelayTime <= 0.0f)
        {
            PlayerCamera_EndKickCinematic();
            PlayerCamera_StartShake(0.3f, 0.8f);

            BallPlayer_Kick(g_PendingDir, g_PendingPower, g_PendingLift, g_PendingCurve);

            g_KickPending = false;
        }

        return;
    }

    bool hasInput =
        KeyLogger_IsPressed(KK_W) ||
        KeyLogger_IsPressed(KK_A) ||
        KeyLogger_IsPressed(KK_S) ||
        KeyLogger_IsPressed(KK_D);

    // DRIBBLE (moved)
    BallControl_UpdateDribble(dt, g_Ball, g_IsKicked, g_HitStopActive);

    // KICK CHARGE (moved)
    BallKickRelease rel = BallControl_UpdateKickCharge(dt, g_IsKicked, g_HitStopActive);
    if (rel.released)
    {
        if (rel.doCinematic)
        {
            g_PendingDir = rel.dir;
            g_PendingPower = rel.power;
            g_PendingLift = rel.lift;
            g_PendingCurve = rel.curve;

            g_KickPending = true;
            g_KickDelayTime = KICK_CINEMATIC_HOLD;

            PlayerCamera_SnapCloseNow();
            return;
        }
        else
        {
            BallPlayer_Kick(rel.dir, rel.power, rel.lift, rel.curve);
        }
    }

    g_PrevBallPos = g_Ball.position;

    // NEW: Air curve control for 1.5 seconds after release (moved)
    BallControl_UpdateAirCurveControl(dt, g_Ball, g_IsKicked);

    // ------------------------------------------------------------
    // CURVE PROJECTILE TRAIL (ONLY when curving mid-air)
    // Uses Y spin as "curve is happening"
    // ------------------------------------------------------------
    if (g_IsKicked && !g_Ball.onGround)
    {
        float spinY = XMVectorGetY(g_Ball.angularVelocity);

        float spinAbs = fabsf(spinY);

        // much lower threshold
        const float SPIN_THRESHOLD = 0.05f;  // was 0.03f

        if (spinAbs > SPIN_THRESHOLD)
        {
            // scale spawn density based on spin
            float intensity = spinAbs / 0.35f;
            if (intensity > 1.0f) intensity = 1.0f;

            g_TrajSpawnAcc += dt;

            // slight curve = sparse trail
            // strong curve = dense trail
            float SPAWN_STEP = 0.012f - (0.008f * intensity);
            if (SPAWN_STEP < 0.006f) SPAWN_STEP = 0.006f;

            // optional: trail behind ball (looks more projectile)
            XMVECTOR vel = XMVectorSet(g_Ball.velocity.x, g_Ball.velocity.y, g_Ball.velocity.z, 0);
            XMVECTOR velLen = XMVector3Length(vel);
            float vlen = XMVectorGetX(velLen);

            XMVECTOR velXZ = XMVectorSet(g_Ball.velocity.x, 0.0f, g_Ball.velocity.z, 0);
            float vlenXZ = XMVectorGetX(XMVector3Length(velXZ));
            XMVECTOR dir = (vlenXZ > 0.001f) ? XMVector3Normalize(velXZ) : XMVectorSet(0, 0, 1, 0);

            while (g_TrajSpawnAcc >= SPAWN_STEP)
            {
                XMFLOAT3 p = g_Ball.position;
                p.x += g_Ball.velocity.x * dt;
                p.y += g_Ball.velocity.y * dt;
                p.z += g_Ball.velocity.z * dt;

                // offset slightly back + up
                p.x -= XMVectorGetX(dir) * 0.12f;
                p.z -= XMVectorGetZ(dir) * 0.12f;
                p.y += 0.05f;
                // MANUAL NUDGE (tune these until it sits perfect)
                p.x += 0.5f;   // + = right, - = left
                p.y += 0.20f;   // + = up,   - = down
                p.z += 0.00f;   // + = forward, - = backward

                XMFLOAT4 col = { 0.85f, 0.92f, 1.0f, 0.85f }; // a bit stronger
                float size = 0.28f;                           // bigger
                double life = 0.35;                           // stays longer

                Trajectory3d_Create(p, col, size, life);

                g_TrajSpawnAcc -= SPAWN_STEP;
            }
        }
        else
        {
            g_TrajSpawnAcc = 0.0f;
        }
    }
    else
    {
        g_TrajSpawnAcc = 0.0f;
    }

    // physics ONCE
    BallPhysics_Update(g_Ball, dt);

    // score check AFTER physics
    if (!g_Scored && GoalCollision_DidCrossGoalLine(g_PrevBallPos, g_Ball.position, BALL_RADIUS))
    {
        g_Scored = true;
        //UI_GoalAnim_Play();
        Game_OnGoalScored();
    }

    // GOAL COLLISION
    Goal_HandleBallCollision(g_Ball.position, g_Ball.velocity, BALL_RADIUS);

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
    // draw trail first so ball is on top
    Trajetory3d_Draw();

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

    Trajetory3d_Finalize();
}

XMFLOAT3 BallPlayer_GetPosition()
{
    return g_Ball.position;
}

XMFLOAT3 BallPlayer_GetAimDirection()
{
    return BallControl_GetAimDirection(g_IsKicked);
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
    return BallControl_GetKickCharge();
}

bool BallPlayer_IsCharging()
{
    return BallControl_IsCharging();
}

float BallPlayer_GetKickMaxPower()
{
    return BallControl_GetKickMaxPower();
}

bool BallPlayer_IsKicked()
{
    return g_IsKicked;
}