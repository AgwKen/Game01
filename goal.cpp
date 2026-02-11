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

// DEBUG FLAGS (THIS IS WHAT YOU WANTED)
static bool g_ShowPoleDebug = true;   // show poles
static bool g_ShowNetDebug = false;  // hide nets

// Back net tuning
static float g_BackNetOffsetX = -2.5f;
static float g_BackNetOffsetZ = 2.4f;
static float g_BackNetWidth = 5.0f;

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

    float space = 0.2f;   // move nets outward

    AABB LeftNet;
    LeftNet.min = {
        g_GoalCollisionPosition.x - (width / 2) - space,
        baseY,
        g_GoalCollisionPosition.z
    };

    LeftNet.max = {
        g_GoalCollisionPosition.x - (width / 2) + depth - space,
        baseY + height,
        g_GoalCollisionPosition.z + g_BackNetOffsetZ
    };

    AABB RightNet;
    RightNet.min = {
        g_GoalCollisionPosition.x + (width / 2) - depth + space,
        baseY,
        g_GoalCollisionPosition.z
    };

    RightNet.max = {
        g_GoalCollisionPosition.x + (width / 2) + space,
        baseY + height,
        g_GoalCollisionPosition.z + g_BackNetOffsetZ
    };

    // BACK NET
    AABB BackNet;
    BackNet.min = {
        g_GoalCollisionPosition.x + g_BackNetOffsetX,
        baseY,
        g_GoalCollisionPosition.z + g_BackNetOffsetZ
    };

    BackNet.max = {
        g_GoalCollisionPosition.x + g_BackNetOffsetX + g_BackNetWidth,
        baseY + height,
        g_GoalCollisionPosition.z + g_BackNetOffsetZ + depth
    };

    // ORDER MATTERS (FIRST 3 ARE POLES)
    out.push_back(left);
    out.push_back(right);
    out.push_back(top);

    // LAST 3 ARE NETS
    out.push_back(BackNet);
    out.push_back(LeftNet);
    out.push_back(RightNet);
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

    std::vector<AABB> boxes;
    GetGoalCollisionBoxes(boxes);

    // ----- DRAW POLES ONLY -----
    if (g_ShowPoleDebug)
    {
        for (int i = 0; i < 3; i++)
        {
            AABB& box = boxes[i];

            XMFLOAT3 center = {
                (box.min.x + box.max.x) * 0.5f,
                (box.min.y + box.max.y) * 0.5f,
                (box.min.z + box.max.z) * 0.5f
            };

            XMFLOAT3 size = {
                box.max.x - box.min.x,
                box.max.y - box.min.y,
                box.max.z - box.min.z
            };

            XMMATRIX debugWorld =
                XMMatrixScaling(size.x, size.y, size.z) *
                XMMatrixTranslation(center.x, center.y, center.z);

            Direct3D_SetAlphaBlendState();
            CUBE_Draw(-1, debugWorld);
            Direct3D_SetDefaultBlendState();
        }
    }

    // ----- DRAW NETS ONLY IF ENABLED -----
    if (g_ShowNetDebug)
    {
        for (int i = 3; i < boxes.size(); i++)
        {
            AABB& box = boxes[i];

            XMFLOAT3 center = {
                (box.min.x + box.max.x) * 0.5f,
                (box.min.y + box.max.y) * 0.5f,
                (box.min.z + box.max.z) * 0.5f
            };

            XMFLOAT3 size = {
                box.max.x - box.min.x,
                box.max.y - box.min.y,
                box.max.z - box.min.z
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

bool Goal_CheckScored(const XMFLOAT3& ballPos, float ballRadius)
{
    return false;
}
