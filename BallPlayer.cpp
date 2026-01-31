#include "BallPlayer.h"
#include "direct3d.h"
#include "model.h"
#include "key_logger.h"
#include "terrain.h"
#include <DirectXMath.h>
#include <cmath>
#include "shader3d.h"

using namespace DirectX;

// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------
static MODEL* g_BallModel = nullptr;

// -----------------------------------------------------------------------------
// Ball state (CENTER position)
// -----------------------------------------------------------------------------
static XMFLOAT3 g_Position = { 0.0f, 5.0f, 0.0f };
static XMFLOAT3 g_PrevPosition = { 0.0f, 5.0f, 0.0f };
static XMFLOAT3 g_Velocity = { 0.0f, 0.0f, 0.0f };

// -----------------------------------------------------------------------------
// SIZE CONTROL
// -----------------------------------------------------------------------------
static constexpr float BALL_COLLISION_RADIUS = 0.4f;
static constexpr float BALL_VISUAL_RADIUS = 0.05f;

// Physics
static constexpr float GRAVITY = -25.0f;

// Movement tuning
static constexpr float MOVE_ACCEL = 12.0f;
static constexpr float MOVE_DAMPING = 0.92f;
static constexpr float MAX_SPEED = 6.0f;

// -----------------------------------------------------------------------------
// Matrices
// -----------------------------------------------------------------------------
static XMMATRIX g_Rotation = XMMatrixIdentity();
static XMMATRIX g_World = XMMatrixIdentity();
static XMMATRIX g_ModelCorrection = XMMatrixIdentity(); // 🔥 KEY FIX

// -----------------------------------------------------------------------------
// Initialize
// -----------------------------------------------------------------------------
void BallPlayer_Initialize(const XMFLOAT3& startPos, float /*unused*/)
{
    g_Position = startPos;
    g_Position.y = 5.0f;
    g_PrevPosition = g_Position;
    g_Velocity = { 0,0,0 };

    g_Rotation = XMMatrixIdentity();
    g_World = XMMatrixIdentity();

    // 🔥 MODEL FACING FIX
    // Model is facing the camera (backwards),
    // rotate 180° so model forward == +Z
    g_ModelCorrection = XMMatrixRotationY(XM_PI);

    g_BallModel = ModelLoad("Resources/Model/work3.fbx", 1.0f);
}

// -----------------------------------------------------------------------------
// Update
// -----------------------------------------------------------------------------
void BallPlayer_Update(double elapsedTime)
{
    float dt = static_cast<float>(elapsedTime);

    // ---------------- INPUT ----------------
    XMFLOAT3 input = { 0,0,0 };
    if (KeyLogger_IsPressed(KK_W)) input.z += 1.0f;
    if (KeyLogger_IsPressed(KK_S)) input.z -= 1.0f;
    if (KeyLogger_IsPressed(KK_A)) input.x -= 1.0f;
    if (KeyLogger_IsPressed(KK_D)) input.x += 1.0f;

    XMVECTOR inputVec = XMVectorSet(input.x, 0.0f, input.z, 0.0f);
    if (XMVectorGetX(XMVector3LengthSq(inputVec)) > 0.0001f)
        inputVec = XMVector3Normalize(inputVec);
    else
        inputVec = XMVectorZero();

    // ---------------- ACCELERATION ----------------
    g_Velocity.x += XMVectorGetX(inputVec) * MOVE_ACCEL * dt;
    g_Velocity.z += XMVectorGetZ(inputVec) * MOVE_ACCEL * dt;

    // ---------------- DAMPING ----------------
    g_Velocity.x *= MOVE_DAMPING;
    g_Velocity.z *= MOVE_DAMPING;

    // ---------------- SPEED CLAMP ----------------
    XMVECTOR velXZ = XMVectorSet(g_Velocity.x, 0, g_Velocity.z, 0);
    float speed = XMVectorGetX(XMVector3Length(velXZ));

    if (speed > MAX_SPEED)
    {
        velXZ = XMVector3Normalize(velXZ) * MAX_SPEED;
        g_Velocity.x = XMVectorGetX(velXZ);
        g_Velocity.z = XMVectorGetZ(velXZ);
    }

    // ---------------- GRAVITY ----------------
    g_Velocity.y += GRAVITY * dt;

    // ---------------- MOVE ----------------
    g_PrevPosition = g_Position;
    g_Position.x += g_Velocity.x * dt;
    g_Position.y += g_Velocity.y * dt;
    g_Position.z += g_Velocity.z * dt;

    // ---------------- TERRAIN COLLISION ----------------
    float groundHeight = Mesh_GetHeightAt(g_Position.x, g_Position.z);
    float ballBottom = g_Position.y - BALL_COLLISION_RADIUS;

    if (ballBottom < groundHeight)
    {
        g_Position.y = groundHeight + BALL_COLLISION_RADIUS;

        if (g_Velocity.y < 0.0f)
            g_Velocity.y = 0.0f;
    }

    // ---------------- ROLLING ROTATION ----------------
    XMFLOAT3 delta = {
        g_Position.x - g_PrevPosition.x,
        0.0f,
        g_Position.z - g_PrevPosition.z
    };

    XMVECTOR deltaVec = XMVectorSet(
        g_Position.x - g_PrevPosition.x,
        0.0f,
        g_Position.z - g_PrevPosition.z,
        0.0f
    );

    float distance = XMVectorGetX(XMVector3Length(deltaVec));

    XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
    XMVECTOR moveDir = XMVector3Normalize(deltaVec);

    // RIGHT-HAND RULE FIX (important)
    XMVECTOR rollAxis = XMVector3Normalize(
        XMVector3Cross(moveDir, worldUp)
    );


    if (distance > 0.00001f)
    {
        // Correct axis: UP × movement direction
        XMVECTOR axis = XMVector3Normalize(
            XMVector3Cross(XMVectorSet(0, 1, 0, 0), deltaVec)
        );

        float angle = distance / BALL_COLLISION_RADIUS;
        g_Rotation = g_Rotation * XMMatrixRotationAxis(rollAxis, angle);
    }


    // ---------------- WORLD MATRIX ----------------
    XMMATRIX scale = XMMatrixScaling(
        BALL_VISUAL_RADIUS,
        BALL_VISUAL_RADIUS,
        BALL_VISUAL_RADIUS
    );

    g_World =
        scale *
        g_Rotation *          // ← roll in world space first
        g_ModelCorrection *   // ← then fix model facing
        XMMatrixTranslation(
            g_Position.x,
            g_Position.y,
            g_Position.z
        );

}

// -----------------------------------------------------------------------------
// Draw
// -----------------------------------------------------------------------------
void BallPlayer_Draw()
{
    if (!g_BallModel) return;
    ModelDraw(g_BallModel, g_World);
}

// -----------------------------------------------------------------------------
// Finalize
// -----------------------------------------------------------------------------
void BallPlayer_Finalize()
{
    if (g_BallModel)
    {
        ModelRelease(g_BallModel);
        g_BallModel = nullptr;
    }
}

// -----------------------------------------------------------------------------
// Utility
// -----------------------------------------------------------------------------
XMFLOAT3 BallPlayer_GetPosition()
{
    return g_Position;
}

float BallPlayer_GetRadius()
{
    return BALL_COLLISION_RADIUS;
}
