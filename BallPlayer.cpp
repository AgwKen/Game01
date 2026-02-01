#include "BallPlayer.h"
#include "direct3d.h"
#include "model.h"
#include "key_logger.h"
#include "terrain.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

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
static XMVECTOR g_AngularVelocity = XMVectorZero();

// ============================================================================
// PHYSICS CONSTANTS
// ============================================================================
static constexpr float BALL_RADIUS = 0.4f;
static constexpr float BALL_SCALE = 0.05f;
static constexpr float GRAVITY = -25.0f;

// Ground behavior
static constexpr float BASE_FRICTION = 6.0f;
static constexpr float STOP_SPEED = 0.15f;
static constexpr float STOP_SPIN = 0.2f;

// Bounce
static constexpr float BOUNCE_RESTITUTION = 0.35f;
static constexpr float BOUNCE_MIN_Y = 2.2f;
static constexpr float SPIN_LOSS_ON_BOUNCE = 0.7f;

// ============================================================================
// KICK SYSTEM
// ============================================================================
static bool  g_IsKicked = false;
static bool  g_IsChargingKick = false;
static float g_KickCharge = 0.0f;

static constexpr float KICK_MIN_POWER = 8.0f;
static constexpr float KICK_MAX_POWER = 30.0f;
static constexpr float KICK_CHARGE_SPEED = 100.0f;

// Air curve (one-time)
static bool  g_AirCurveUsed = false;
static float g_AirCurveDir = 0.0f;
static float g_AirCurveTime = 0.0f;
static constexpr float AIR_CURVE_DURATION = 0.35f;

// ============================================================================
// MATRICES
// ============================================================================
static XMMATRIX g_Rotation = XMMatrixIdentity();
static XMMATRIX g_ModelCorrection = XMMatrixRotationY(XM_PI);
static XMMATRIX g_World = XMMatrixIdentity();

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
    XMVECTOR forward = XMVector3Normalize(XMVectorSet(dir.x, 0, dir.z, 0));

    g_Velocity.x += XMVectorGetX(forward) * power;
    g_Velocity.z += XMVectorGetZ(forward) * power;
    g_Velocity.y += lift;

    XMVECTOR side = XMVector3Normalize(
        XMVector3Cross(forward, XMVectorSet(0, 1, 0, 0))
    );

    g_AngularVelocity += side * curve;

    g_IsKicked = true;
    g_AirCurveUsed = false;
    g_AirCurveDir = 0.0f;
    g_AirCurveTime = 0.0f;
}

// ============================================================================
// INIT
// ============================================================================
void BallPlayer_Initialize(const XMFLOAT3& startPos, float)
{
    g_Position = startPos;
    g_Position.y = 5.0f;
    g_PrevPosition = g_Position;

    g_Velocity = { 0,0,0 };
    g_AngularVelocity = XMVectorZero();

    g_IsKicked = false;
    g_IsChargingKick = false;
    g_KickCharge = 0.0f;

    g_Rotation = XMMatrixIdentity();

    g_BallModel = ModelLoad("Resources/Model/work3.fbx", 1.0f);
}

// ============================================================================
// UPDATE
// ============================================================================
void BallPlayer_Update(double elapsedTime)
{
    float dt = (float)elapsedTime;

    // ------------------------------------------------------------------------
    // DRIBBLE CONTROL (GROUND, BEFORE KICK)
    // ------------------------------------------------------------------------
    if (!g_IsKicked)
    {
        XMFLOAT3 in = { 0,0,0 };
        if (KeyLogger_IsPressed(KK_W)) in.z += 1;
        if (KeyLogger_IsPressed(KK_S)) in.z -= 1;
        if (KeyLogger_IsPressed(KK_A)) in.x -= 1;
        if (KeyLogger_IsPressed(KK_D)) in.x += 1;

        XMVECTOR dir = XMVectorSet(in.x, 0, in.z, 0);
        if (XMVectorGetX(XMVector3LengthSq(dir)) > 0.001f)
        {
            dir = XMVector3Normalize(dir);
            g_Velocity.x += XMVectorGetX(dir) * 18.0f * dt;
            g_Velocity.z += XMVectorGetZ(dir) * 18.0f * dt;
        }
    }

    // ------------------------------------------------------------------------
    // KICK CHARGING
    // ------------------------------------------------------------------------
    if (!g_IsKicked)
    {
        if (KeyLogger_IsPressed(KK_SPACE))
        {
            g_IsChargingKick = true;
            g_KickCharge += KICK_CHARGE_SPEED * dt;
            g_KickCharge = std::min(g_KickCharge, KICK_MAX_POWER);
        }
        else if (g_IsChargingKick)
        {
            float power = std::max(g_KickCharge, KICK_MIN_POWER);
            BallPlayer_Kick({ 0,0,1 }, power, power * 0.35f, power * 0.3f);
            g_KickCharge = 0.0f;
            g_IsChargingKick = false;
        }
    }

    // ------------------------------------------------------------------------
    // GRAVITY
    // ------------------------------------------------------------------------
    g_Velocity.y += GRAVITY * dt;

    // ------------------------------------------------------------------------
    // MOVE
    // ------------------------------------------------------------------------
    g_PrevPosition = g_Position;
    g_Position.x += g_Velocity.x * dt;
    g_Position.y += g_Velocity.y * dt;
    g_Position.z += g_Velocity.z * dt;

 
    // ------------------------------------------------------------------------
    // TERRAIN COLLISION (REAL PHYSICS)
    // ------------------------------------------------------------------------
    float ground = Mesh_GetHeightAt(g_Position.x, g_Position.z);
    float bottom = g_Position.y - BALL_RADIUS;
    bool onGround = false;

    if (bottom < ground)
    {
        g_Position.y = ground + BALL_RADIUS;

        XMFLOAT3 n = GetTerrainNormal(g_Position.x, g_Position.z);
        XMVECTOR normal = XMLoadFloat3(&n);
        XMVECTOR vel = XMLoadFloat3(&g_Velocity);

        float vn = XMVectorGetX(XMVector3Dot(vel, normal));
        if (vn < 0.0f)
            vel -= normal * vn;

        XMStoreFloat3(&g_Velocity, vel);

        if (n.y > 0.4f) onGround = true;
    }


    // ------------------------------------------------------------------------
    // AIR CURVE (ONE-TIME)
    // ------------------------------------------------------------------------
    if (!onGround)
    {
        if (!g_AirCurveUsed)
        {
            if (KeyLogger_IsPressed(KK_A)) { g_AirCurveUsed = true; g_AirCurveDir = -1; }
            if (KeyLogger_IsPressed(KK_D)) { g_AirCurveUsed = true; g_AirCurveDir = 1; }
        }

        if (g_AirCurveUsed && g_AirCurveTime < AIR_CURVE_DURATION)
        {
            g_AirCurveTime += dt;
            XMVECTOR vel = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
            float speed = XMVectorGetX(XMVector3Length(vel));

            if (speed > 0.01f)
            {
                XMMATRIX turn = XMMatrixRotationY(g_AirCurveDir * 2.0f * dt);
                vel = XMVector3TransformNormal(vel, turn);
                vel = XMVector3Normalize(vel) * speed;
                g_Velocity.x = XMVectorGetX(vel);
                g_Velocity.z = XMVectorGetZ(vel);
            }
        }
    }

    // ------------------------------------------------------------------------
    // MAGNUS + SPIN DAMP
    // ------------------------------------------------------------------------
    XMVECTOR velXZ = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
    if (XMVectorGetX(XMVector3Length(g_AngularVelocity)) > 0.01f)
    {
        XMVECTOR mag = XMVector3Cross(g_AngularVelocity, velXZ) * 0.015f;
        g_Velocity.x += XMVectorGetX(mag);
        g_Velocity.z += XMVectorGetZ(mag);
        g_AngularVelocity *= 0.99f;
    }

    // ------------------------------------------------------------------------
    // GROUND FRICTION + STOP
    // ------------------------------------------------------------------------
    if (onGround)
    {
        float speed = XMVectorGetX(XMVector3Length(velXZ));
        float spin = XMVectorGetX(XMVector3Length(g_AngularVelocity));

        XMFLOAT3 normal = GetTerrainNormal(g_Position.x, g_Position.z);
        float slope = 1.0f - normal.y;

        float friction = BASE_FRICTION * (1.0f - slope * 0.8f);
        speed = std::max(0.0f, speed - friction * dt);

        if (speed > 0.0f)
        {
            velXZ = XMVector3Normalize(velXZ) * speed;
            g_Velocity.x = XMVectorGetX(velXZ);
            g_Velocity.z = XMVectorGetZ(velXZ);
        }

        if (spin < STOP_SPIN && speed < STOP_SPEED)
        {
            g_Velocity = { 0,0,0 };
            g_AngularVelocity = XMVectorZero();
            g_IsKicked = false;
        }
    }

    // ------------------------------------------------------------------------
    // VISUAL ROLL
    // ------------------------------------------------------------------------
    if (XMVectorGetX(XMVector3Length(velXZ)) > 0.001f)
    {
        XMVECTOR axis = XMVector3Normalize(
            XMVector3Cross(XMVector3Normalize(velXZ), XMVectorSet(0, 1, 0, 0))
        );

        g_Rotation *= XMMatrixRotationAxis(
            axis,
            XMVectorGetX(XMVector3Length(velXZ)) * dt / BALL_RADIUS
        );
    }

    // ------------------------------------------------------------------------
    // WORLD
    // ------------------------------------------------------------------------
    g_World =
        XMMatrixScaling(BALL_SCALE, BALL_SCALE, BALL_SCALE) *
        g_Rotation *
        g_ModelCorrection *
        XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);
}

// ============================================================================
// DRAW / CLEANUP
// ============================================================================
void BallPlayer_Draw()
{
    if (g_BallModel) ModelDraw(g_BallModel, g_World);
}

void BallPlayer_Finalize()
{
    if (g_BallModel) ModelRelease(g_BallModel);
    g_BallModel = nullptr;
}

XMFLOAT3 BallPlayer_GetPosition() { return g_Position; }
float BallPlayer_GetRadius() { return BALL_RADIUS; }
