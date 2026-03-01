#include "GoalCollision.h"
#include "terrain.h"
#include "Goal.h"

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

static XMFLOAT3 g_GoalCollisionBase = { 9.3f, 0.0f, 1.3f };
static XMFLOAT3 g_GoalModelBase = { 7.0f, 0.0f, 2.5f };

static bool g_BackNetHit = false;

static void BuildGoalCollision(std::vector<AABB>& out);

static bool IsInsideGoalMouth(const XMFLOAT3& p, float r, const std::vector<AABB>& boxes)
{
    const AABB& left = boxes[0];
    const AABB& right = boxes[1];
    const AABB& top = boxes[2];

    // X range between inner faces of poles
    float minX = left.max.x;
    float maxX = right.min.x;

    // Y range under crossbar
    float minY = left.min.y;
    float maxY = top.min.y;

    // Z goal line (front of goal). Poles are thin in Z, so use their middle/front.
    float goalLineZ = (left.min.z + left.max.z) * 0.5f;

    return
        p.x > minX - r && p.x < maxX + r &&
        p.y > minY - r && p.y < maxY + r &&
        p.z > goalLineZ; // behind the line
}


bool GoalCollision_IsBallInsideGoal(const XMFLOAT3& ballPos, float radius)
{
    std::vector<AABB> boxes;
    BuildGoalCollision(boxes);

    // BackNet is index 3
    const AABB& back = boxes[3];

    bool inside =
        ballPos.x > back.min.x - radius &&
        ballPos.x < back.max.x + radius &&
        ballPos.y > back.min.y - radius &&
        ballPos.y < back.max.y + radius &&
        ballPos.z > back.min.z - radius &&
        ballPos.z < back.max.z + radius;

    return inside;
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
    extern XMFLOAT3 Goal_GetWorldPosition();

    XMFLOAT3 goalPos = Goal_GetWorldPosition();

    XMFLOAT3 delta =
    {
        goalPos.x - g_GoalModelBase.x,
        goalPos.y - g_GoalModelBase.y,
        goalPos.z - g_GoalModelBase.z
    };

    XMFLOAT3 p =
    {
        g_GoalCollisionBase.x + delta.x,
        g_GoalCollisionBase.y + delta.y,
        g_GoalCollisionBase.z + delta.z
    };

    float baseY = g_GoalWorldBaseY;


    float scale = g_GoalCollisionScale;

    float width = 15.2f * scale;
    float height = 8.0f * scale;
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
    if (dist < 0.0001f)
    {
        if (isTopNet)
        {
            // put ball definitely ABOVE the top net box
            pos.y = box.max.y + radius + 0.001f;

            // stop falling so it won't sink back in next frame
            if (vel.y < 0.0f) vel.y = 0.0f;

            // small friction so it settles
            vel.x *= 0.8f;
            vel.z *= 0.8f;

            return true;
        }

        // for other boxes, ignore this rare degenerate case
        return false;
    }

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
                if (vel.y < 0.0f) vel.y = 0.0f;
                pos.y += penetration + 0.002f;
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
        {
            hit = true;

            if (i == 3) // BackNet index
            {
                g_BackNetHit = true;
            }
        }
    }

    return hit;
}

// ------------------------------------------------------------
void GoalCollision_GetDebugBoxes(std::vector<AABB>& out)
{
    BuildGoalCollision(out);
}
bool GoalCollision_BackNetTouched()
{
    bool touched = g_BackNetHit;
    g_BackNetHit = false;
    return touched;
}
void GoalCollision_ClearBackNetHit()
{
    g_BackNetHit = false;
}
bool GoalCollision_DidCrossGoalLine(const XMFLOAT3& prev, const XMFLOAT3& now, float r)
{
    std::vector<AABB> boxes;
    BuildGoalCollision(boxes);

    const AABB& left = boxes[0];
    float goalLineZ = (left.min.z + left.max.z) * 0.5f;

    // must cross from front -> behind
    bool crossed = (prev.z <= goalLineZ && now.z > goalLineZ);

    if (!crossed) return false;

    // must be inside mouth at the moment it crosses
    return IsInsideGoalMouth(now, r, boxes);
}

bool GoalCollision_GetGoalMouthTarget(DirectX::XMFLOAT3& outTarget)
{
    std::vector<AABB> boxes;
    BuildGoalCollision(boxes);

    if (boxes.size() < 3) return false;

    const AABB& left = boxes[0];
    const AABB& right = boxes[1];
    const AABB& top = boxes[2];

    // Inside between poles
    float minX = left.max.x;
    float maxX = right.min.x;

    // Under crossbar
    float minY = left.min.y;
    float maxY = top.min.y;

    // Goal line Z (same logic you used)
    float goalLineZ = (left.min.z + left.max.z) * 0.5f;

    // Put target slightly BEHIND the goal line so it truly goes "in"
    float targetZ = goalLineZ + 0.35f;

    outTarget.x = (minX + maxX) * 0.5f;
    outTarget.y = (minY + maxY) * 0.5f;   // mid height of mouth
    outTarget.z = targetZ;

    return true;
}
