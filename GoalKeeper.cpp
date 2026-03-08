#include "GoalKeeper.h"
#include "Goal.h"
#include "cube.h"
#include "terrain.h"
#include "direct3d.h"
#include "texture.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

using namespace DirectX;

// ============================================================
// SETTINGS
// ============================================================
static XMFLOAT3 g_KeeperPos = { 0,0,0 };
static XMFLOAT3 g_KeeperScale = { 0.8f, 1.4f, 0.4f };

// movement inside goal mouth
static float g_MoveSpeed = 3.0f;
static float g_LeftLimit = -0.5f;
static float g_RightLimit = 4.0f;
static int   g_MoveDir = 1; // 1 = right, -1 = left

// placement relative to goal
static float g_ForwardOffset = -1.5f;  // slightly in front of goal line
static float g_FeetOffsetY = 0.8f;     // half of keeper height

// texture
static int g_KeeperTex = -1;

// ============================================================
// INTERNAL HELPERS
// ============================================================
static float ClampFloat(float v, float mn, float mx)
{
    if (v < mn) return mn;
    if (v > mx) return mx;
    return v;
}

static void UpdateKeeperPlacementFromGoal()
{
    XMFLOAT3 goalPos = Goal_GetWorldPosition();

    // Keep goalkeeper aligned to goal position
    g_KeeperPos.x = ClampFloat(g_KeeperPos.x, goalPos.x + g_LeftLimit, goalPos.x + g_RightLimit);
    g_KeeperPos.z = goalPos.z + g_ForwardOffset;

    float groundY = Mesh_GetHeightAt(g_KeeperPos.x, g_KeeperPos.z);
    g_KeeperPos.y = groundY + g_FeetOffsetY;
}

static bool SphereAABBIntersect(
    const XMFLOAT3& sphereCenter,
    float sphereRadius,
    const AABB& box)
{
    float cx = ClampFloat(sphereCenter.x, box.min.x, box.max.x);
    float cy = ClampFloat(sphereCenter.y, box.min.y, box.max.y);
    float cz = ClampFloat(sphereCenter.z, box.min.z, box.max.z);

    float dx = sphereCenter.x - cx;
    float dy = sphereCenter.y - cy;
    float dz = sphereCenter.z - cz;

    float distSq = dx * dx + dy * dy + dz * dz;
    return distSq <= (sphereRadius * sphereRadius);
}

static AABB GetKeeperAABB()
{
    XMFLOAT3 half =
    {
        g_KeeperScale.x * 0.5f,
        g_KeeperScale.y * 0.5f,
        g_KeeperScale.z * 0.5f
    };

    AABB box;
    box.min = { g_KeeperPos.x - half.x, g_KeeperPos.y - half.y, g_KeeperPos.z - half.z };
    box.max = { g_KeeperPos.x + half.x, g_KeeperPos.y + half.y, g_KeeperPos.z + half.z };
    return box;
}

// ============================================================
// PUBLIC
// ============================================================
void GoalKeeper_Initialize()
{
    XMFLOAT3 goalPos = Goal_GetWorldPosition();
    g_KeeperPos = { goalPos.x, 0.0f, goalPos.z + g_ForwardOffset };
    g_MoveDir = 1;

    // load texture here
    g_KeeperTex = Texture_Load(L"Texture/keeper.png");

    UpdateKeeperPlacementFromGoal();
}

void GoalKeeper_Finalize()
{
    // if your engine has texture release, use it here
    // example:
    // if (g_KeeperTex >= 0) Texture_Unload(g_KeeperTex);

    g_KeeperTex = -1;
}

void GoalKeeper_Update(double dt)
{
    XMFLOAT3 goalPos = Goal_GetWorldPosition();

    g_KeeperPos.x += g_MoveDir * g_MoveSpeed * (float)dt;

    float minX = goalPos.x + g_LeftLimit;
    float maxX = goalPos.x + g_RightLimit;

    if (g_KeeperPos.x > maxX)
    {
        g_KeeperPos.x = maxX;
        g_MoveDir = -1;
    }
    else if (g_KeeperPos.x < minX)
    {
        g_KeeperPos.x = minX;
        g_MoveDir = 1;
    }

    g_KeeperPos.z = goalPos.z + g_ForwardOffset;

    float groundY = Mesh_GetHeightAt(g_KeeperPos.x, g_KeeperPos.z);
    g_KeeperPos.y = groundY + g_FeetOffsetY;
}

void GoalKeeper_Render()
{
    XMMATRIX world =
        XMMatrixScaling(g_KeeperScale.x, g_KeeperScale.y, g_KeeperScale.z) *
        XMMatrixTranslation(g_KeeperPos.x, g_KeeperPos.y, g_KeeperPos.z);

    Direct3D_SetAlphaBlendState();
    CUBE_Draw(g_KeeperTex, world);
    Direct3D_SetDefaultBlendState();
}

bool GoalKeeper_HandleBallCollision(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius)
{
    AABB box = GetKeeperAABB();

    if (!SphereAABBIntersect(ballPos, ballRadius, box))
        return false;

    float distFront = fabsf(ballPos.z - box.min.z);
    float distBack = fabsf(ballPos.z - box.max.z);
    float distLeft = fabsf(ballPos.x - box.min.x);
    float distRight = fabsf(ballPos.x - box.max.x);
    float distTop = fabsf(ballPos.y - box.max.y);

    float minDist = distFront;
    int hitFace = 0; // 0 front, 1 back, 2 left, 3 right, 4 top

    if (distBack < minDist) { minDist = distBack; hitFace = 1; }
    if (distLeft < minDist) { minDist = distLeft; hitFace = 2; }
    if (distRight < minDist) { minDist = distRight; hitFace = 3; }
    if (distTop < minDist) { minDist = distTop; hitFace = 4; }

    switch (hitFace)
    {
    case 0:
        ballPos.z = box.min.z - ballRadius - 0.01f;
        ballVelocity.z = -fabsf(ballVelocity.z) * 0.75f;
        break;

    case 1:
        ballPos.z = box.max.z + ballRadius + 0.01f;
        ballVelocity.z = fabsf(ballVelocity.z) * 0.75f;
        break;

    case 2:
        ballPos.x = box.min.x - ballRadius - 0.01f;
        ballVelocity.x = -fabsf(ballVelocity.x) * 0.75f;
        break;

    case 3:
        ballPos.x = box.max.x + ballRadius + 0.01f;
        ballVelocity.x = fabsf(ballVelocity.x) * 0.75f;
        break;

    case 4:
        ballPos.y = box.max.y + ballRadius + 0.01f;
        ballVelocity.y = fabsf(ballVelocity.y) * 0.6f;
        break;
    }

    ballVelocity.x *= 0.92f;
    ballVelocity.y *= 0.92f;
    ballVelocity.z *= 0.92f;

    return true;
}

XMFLOAT3 GoalKeeper_GetPosition()
{
    return g_KeeperPos;
}