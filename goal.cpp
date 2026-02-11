#include "Goal.h"
#include "model.h"
#include "terrain.h"
#include "cube.h"
#include "direct3d.h"
#include <vector>
#include <algorithm>
#include <cmath>

using namespace DirectX;

// ------------------------------------------------------------
// Goal Model
// ------------------------------------------------------------
static MODEL* g_GoalModel = nullptr;

// TRANSFORMS (SEPARATED)
static XMFLOAT3 g_GoalModelPosition = { 7.0f, 0.0f, 2.5f };      // visual only
static XMFLOAT3 g_GoalCollisionPosition = { 9.3f, 0.0f, 1.3f };  // collision only

static float g_GoalModelScale = 1.2f;      // only for visual model
static float g_GoalCollisionScale = 0.3f;  // only for AABB sizes

// Visual rotation only
static float g_ModelRotationY = -XM_PIDIV2;

// Height offset so model never sinks underground
static float g_ModelHeightOffset = 1.0f;

// Physics parameters
static float GOAL_BOUNCE = 0.8f;

// Debug
static bool g_ShowGoalDebug = true;

// ------------------------------------------------------------

void Goal_Initialize()
{
    g_GoalModel = ModelLoad("Resources/Model/goal.fbx", 1.0f);
}

void Goal_Finalize()
{
    if (g_GoalModel)
    {
        ModelRelease(g_GoalModel);
        g_GoalModel = nullptr;
    }
}

// ------------------------------------------------------------
// Collision Boxes (USING SEPARATE POSITION)
// ------------------------------------------------------------
static void GetGoalCollisionBoxes(std::vector<AABB>& out)
{
    out.clear();

    float terrainY = Mesh_GetHeightAt(
        g_GoalCollisionPosition.x,
        g_GoalCollisionPosition.z
    );

    float baseY = terrainY + 0.01f;

    float scale = g_GoalCollisionScale;

    float width = 15.2f * scale;
    float height = 8.2f * scale;

    float depth = 0.3f * scale;
    float netDepth = 16.2f * scale;

    // FRONT POSTS
    AABB left;
    left.min = { g_GoalCollisionPosition.x - width / 2 - depth / 2, baseY, g_GoalCollisionPosition.z - depth / 2 };
    left.max = { g_GoalCollisionPosition.x - width / 2 + depth / 2, baseY + height, g_GoalCollisionPosition.z + depth / 2 };

    AABB right;
    right.min = { g_GoalCollisionPosition.x + width / 2 - depth / 2, baseY, g_GoalCollisionPosition.z - depth / 2 };
    right.max = { g_GoalCollisionPosition.x + width / 2 + depth / 2, baseY + height, g_GoalCollisionPosition.z + depth / 2 };

    AABB top;
    top.min = { g_GoalCollisionPosition.x - width / 2, baseY + height - depth / 2, g_GoalCollisionPosition.z - depth / 2 };
    top.max = { g_GoalCollisionPosition.x + width / 2, baseY + height + depth / 2, g_GoalCollisionPosition.z + depth / 2 };

    // SIMPLE BACK WALL FOR NET
    AABB BackNet;
    BackNet.min = { g_GoalCollisionPosition.x - 2.4f, baseY, g_GoalCollisionPosition.z + width / 2 - depth };
    BackNet.max = { g_GoalCollisionPosition.x + netDepth - 2.6f, baseY + height, g_GoalCollisionPosition.z + width / 2 };

    out.push_back(left);
    out.push_back(right);
    out.push_back(top);
    out.push_back(BackNet);
}

// ------------------------------------------------------------
// Sphere vs AABB physics response
// ------------------------------------------------------------
static bool ResolveSphereAABBCollision(
    XMFLOAT3& pos,
    XMFLOAT3& vel,
    float radius,
    const AABB& box)
{
    XMFLOAT3 closest;

    closest.x = std::max(box.min.x, std::min(pos.x, box.max.x));
    closest.y = std::max(box.min.y, std::min(pos.y, box.max.y));
    closest.z = std::max(box.min.z, std::min(pos.z, box.max.z));

    XMFLOAT3 diff = {
        pos.x - closest.x,
        pos.y - closest.y,
        pos.z - closest.z
    };

    float distSq =
        diff.x * diff.x +
        diff.y * diff.y +
        diff.z * diff.z;

    if (distSq > radius * radius)
        return false;

    float dist = sqrtf(distSq);
    if (dist < 0.0001f)
        return false;

    XMFLOAT3 normal = {
        diff.x / dist,
        diff.y / dist,
        diff.z / dist
    };

    float penetration = radius - dist;

    pos.x += normal.x * penetration;
    pos.y += normal.y * penetration;
    pos.z += normal.z * penetration;

    float vn =
        vel.x * normal.x +
        vel.y * normal.y +
        vel.z * normal.z;

    if (vn < 0.0f)
    {
        vel.x -= (1.0f + GOAL_BOUNCE) * vn * normal.x;
        vel.y -= (1.0f + GOAL_BOUNCE) * vn * normal.y;
        vel.z -= (1.0f + GOAL_BOUNCE) * vn * normal.z;
    }

    return true;
}

// ------------------------------------------------------------
// MAIN BALL COLLISION ENTRY
// ------------------------------------------------------------
bool Goal_HandleBallCollision(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius)
{
    if (!g_GoalModel)
        return false;

    std::vector<AABB> boxes;
    GetGoalCollisionBoxes(boxes);

    bool any = false;

    for (auto& box : boxes)
    {
        if (ResolveSphereAABBCollision(
            ballPos,
            ballVelocity,
            ballRadius,
            box))
        {
            any = true;
        }
    }

    return any;
}

// ------------------------------------------------------------
// Drawing (MODEL ALWAYS ABOVE TERRAIN)
// ------------------------------------------------------------
void Goal_Draw()
{
    if (!g_GoalModel)
        return;

    float terrainY = Mesh_GetHeightAt(
        g_GoalModelPosition.x,
        g_GoalModelPosition.z
    );

    // THIS LINE ENSURES MODEL NEVER SINKS
    float finalY = terrainY + (g_ModelHeightOffset * g_GoalModelScale);

    XMMATRIX world =
        XMMatrixScaling(g_GoalModelScale, g_GoalModelScale, g_GoalModelScale) *
        XMMatrixRotationY(g_ModelRotationY) *
        XMMatrixTranslation(
            g_GoalModelPosition.x,
            finalY,
            g_GoalModelPosition.z
        );

    ModelDraw(g_GoalModel, world);

#if defined(DEBUG) || defined(_DEBUG)

    if (g_ShowGoalDebug)
    {
        std::vector<AABB> boxes;
        GetGoalCollisionBoxes(boxes);

        for (auto& box : boxes)
        {
            XMFLOAT3 min = box.min;
            XMFLOAT3 max = box.max;

            XMFLOAT3 center = {
                (min.x + max.x) * 0.5f,
                (min.y + max.y) * 0.5f,
                (min.z + max.z) * 0.5f
            };

            XMFLOAT3 size = {
                max.x - min.x,
                max.y - min.y,
                max.z - min.z
            };

            XMMATRIX debugWorld =
                XMMatrixScaling(size.x, size.y, size.z) *
                XMMatrixTranslation(center.x, center.y, center.z);

            Direct3D_SetAlphaBlendState();
            CUBE_Draw(-1, debugWorld);
            Direct3D_SetDefaultBlendState();
        }
    }

#endif
}

// ------------------------------------------------------------

bool Goal_CheckScored(const XMFLOAT3& ballPos, float ballRadius)
{
    return false;
}
