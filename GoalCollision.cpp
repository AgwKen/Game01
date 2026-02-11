#include "GoalCollision.h"
#include "terrain.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

// Separate collision transform
static XMFLOAT3 g_GoalCollisionPosition = { 9.3f, 0.0f, 1.3f };
static float g_GoalCollisionScale = 0.3f;

// Back net tuning
static float g_BackNetOffsetX = -2.5f;
static float g_BackNetOffsetZ = 2.4f;
static float g_BackNetWidth = 5.0f;

// Physics
static float GOAL_BOUNCE = 0.8f;

void GoalCollision_Initialize()
{
}

void GoalCollision_Finalize()
{
}

static void BuildGoalCollision(std::vector<AABB>& out)
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

    AABB left;
    left.min = { g_GoalCollisionPosition.x - width / 2 - depth / 2, baseY, g_GoalCollisionPosition.z - depth / 2 };
    left.max = { g_GoalCollisionPosition.x - width / 2 + depth / 2, baseY + height, g_GoalCollisionPosition.z + depth / 2 };

    AABB right;
    right.min = { g_GoalCollisionPosition.x + width / 2 - depth / 2, baseY, g_GoalCollisionPosition.z - depth / 2 };
    right.max = { g_GoalCollisionPosition.x + width / 2 + depth / 2, baseY + height, g_GoalCollisionPosition.z + depth / 2 };

    AABB top;
    top.min = { g_GoalCollisionPosition.x - width / 2, baseY + height - depth / 2, g_GoalCollisionPosition.z - depth / 2 };
    top.max = { g_GoalCollisionPosition.x + width / 2, baseY + height + depth / 2, g_GoalCollisionPosition.z + depth / 2 };

    float space = 0.2f;

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

    out.push_back(left);
    out.push_back(right);
    out.push_back(top);

    out.push_back(BackNet);
    out.push_back(LeftNet);
    out.push_back(RightNet);
}


static bool SphereVsAABB(
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

bool GoalCollision_HandleBall(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius)
{
    std::vector<AABB> boxes;
    BuildGoalCollision(boxes);

    bool hit = false;

    for (auto& box : boxes)
    {
        if (SphereVsAABB(ballPos, ballVelocity, ballRadius, box))
            hit = true;
    }

    return hit;
}

void GoalCollision_GetDebugBoxes(std::vector<AABB>& out)
{
    BuildGoalCollision(out);
}
