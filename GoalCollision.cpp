#include "GoalCollision.h"
#include "terrain.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

// ------------------------------------------------------------
// ORIGINAL LOCAL COLLISION PLACEMENT (DO NOT TOUCH THESE)
// ------------------------------------------------------------
static XMFLOAT3 g_GoalCollisionPosition = { 9.3f, 0.0f, 1.3f };
static float g_GoalCollisionScale = 0.3f;

// ------------------------------------------------------------
// GLOBAL OFFSET — MOVES WHOLE GOAL + COLLISION TOGETHER
// ------------------------------------------------------------
XMFLOAT3 g_GoalWorldOffset = { 0,0,0 };

void GoalCollision_SetOffset(const XMFLOAT3& pos)
{
    g_GoalWorldOffset = pos;
}

// ------------------------------------------------------------
// NET TUNING
// ------------------------------------------------------------
static float g_BackNetOffsetX = -2.5f;
static float g_BackNetOffsetZ = 2.4f;
static float g_BackNetWidth = 5.0f;

// Physics
static float GOAL_BOUNCE = 0.8f;

void GoalCollision_Initialize() {}
void GoalCollision_Finalize() {}

float g_GoalWorldBaseY = 0.0f;

// ------------------------------------------------------------
// Build all AABB boxes
// ------------------------------------------------------------
static void BuildGoalCollision(std::vector<AABB>& out)
{
    out.clear();

    // FINAL WORLD POSITION
    XMFLOAT3 p =
    {
        g_GoalCollisionPosition.x + g_GoalWorldOffset.x,
        g_GoalCollisionPosition.y + g_GoalWorldOffset.y,
        g_GoalCollisionPosition.z + g_GoalWorldOffset.z
    };

    float baseY = g_GoalWorldBaseY;


    float scale = g_GoalCollisionScale;

    float width = 15.2f * scale;
    float height = 8.2f * scale;
    float depth = 0.3f * scale;

    // ---- POLES ----
    AABB left;
    left.min = { p.x - width / 2 - depth / 2, baseY, p.z - depth / 2 };
    left.max = { p.x - width / 2 + depth / 2, baseY + height, p.z + depth / 2 };

    AABB right;
    right.min = { p.x + width / 2 - depth / 2, baseY, p.z - depth / 2 };
    right.max = { p.x + width / 2 + depth / 2, baseY + height, p.z + depth / 2 };

    AABB top;
    top.min = { p.x - width / 2, baseY + height - depth / 2, p.z - depth / 2 };
    top.max = { p.x + width / 2, baseY + height + depth / 2, p.z + depth / 2 };

    float space = 0.2f;

    // ---- NETS ----
    AABB LeftNet;
    LeftNet.min = {
        p.x - (width / 2) - space,
        baseY,
        p.z
    };
    LeftNet.max = {
        p.x - (width / 2) + depth - space,
        baseY + height,
        p.z + g_BackNetOffsetZ
    };

    AABB RightNet;
    RightNet.min = {
        p.x + (width / 2) - depth + space,
        baseY,
        p.z
    };
    RightNet.max = {
        p.x + (width / 2) + space,
        baseY + height,
        p.z + g_BackNetOffsetZ
    };

    AABB BackNet;
    BackNet.min = {
        p.x + g_BackNetOffsetX,
        baseY,
        p.z + g_BackNetOffsetZ
    };
    BackNet.max = {
        p.x + g_BackNetOffsetX + g_BackNetWidth,
        baseY + height,
        p.z + g_BackNetOffsetZ + depth
    };

    // ---- TOP NET ----
    AABB TopNet;
    TopNet.min = {
        p.x - (width / 2),
        baseY + height,
        p.z
    };
    TopNet.max = {
        p.x + (width / 2),
        baseY + height + depth,
        p.z + g_BackNetOffsetZ
    };

    // ORDER IMPORTANT
    out.push_back(left);
    out.push_back(right);
    out.push_back(top);
    out.push_back(BackNet);
    out.push_back(LeftNet);
    out.push_back(RightNet);
    out.push_back(TopNet);
}

// ------------------------------------------------------------
// Sphere vs AABB
// ------------------------------------------------------------
static bool SphereVsAABB(
    XMFLOAT3& pos,
    XMFLOAT3& vel,
    float radius,
    const AABB& box,
    bool isNet,
    bool isTopNet)
{
    XMFLOAT3 closest =
    {
        std::max(box.min.x, std::min(pos.x, box.max.x)),
        std::max(box.min.y, std::min(pos.y, box.max.y)),
        std::max(box.min.z, std::min(pos.z, box.max.z))
    };

    XMFLOAT3 diff =
    {
        pos.x - closest.x,
        pos.y - closest.y,
        pos.z - closest.z
    };

    float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
    if (distSq > radius * radius) return false;

    float dist = sqrtf(distSq);
    if (dist < 0.0001f) return false;

    XMFLOAT3 n = { diff.x / dist, diff.y / dist, diff.z / dist };
    float penetration = radius - dist;

    pos.x += n.x * penetration;
    pos.y += n.y * penetration;
    pos.z += n.z * penetration;

    float vn = vel.x * n.x + vel.y * n.y + vel.z * n.z;

    if (vn < 0.0f)
    {
        if (isNet)
        {
            if (isTopNet)
            {
                vel.x *= 0.80f;
                vel.z *= 0.80f;
                vel.y *= 0.95f;
                pos.y += 0.01f;
            }
            else
            {
                vel.x *= 0.15f;
                vel.y *= 0.15f;
                vel.z *= 0.15f;
            }
        }
        else
        {
            vel.x -= (1.0f + GOAL_BOUNCE) * vn * n.x;
            vel.y -= (1.0f + GOAL_BOUNCE) * vn * n.y;
            vel.z -= (1.0f + GOAL_BOUNCE) * vn * n.z;
        }
    }
    return true;
}

// ------------------------------------------------------------
bool GoalCollision_HandleBall(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius)
{
    std::vector<AABB> boxes;
    BuildGoalCollision(boxes);

    bool hit = false;

    for (int i = 0; i < boxes.size(); i++)
    {
        bool isNet = (i >= 3);
        bool isTopNet = (i == 6);

        if (SphereVsAABB(ballPos, ballVelocity, ballRadius, boxes[i], isNet, isTopNet))
            hit = true;
    }

    return hit;
}

// ------------------------------------------------------------
void GoalCollision_GetDebugBoxes(std::vector<AABB>& out)
{
    BuildGoalCollision(out);
}
