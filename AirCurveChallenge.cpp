/*========================================================================================

    AirCurveChallenge.cpp
    FIXED VERSION
    - Stable banana curves
    - Target clamped inside goal
    - Last coin forced into goal

========================================================================================*/

#include "AirCurveChallenge.h"

#include "coin.h"
#include "sprite_anim.h"
#include "CoinScore.h"
#include "GoalCollision.h"
#include "terrain.h"
#include "Goal.h"

#include <vector>
#include <cmath>
#include <cstdlib>
#include "BallPlayer.h"

using namespace DirectX;

extern std::vector<Coin> g_Coins;
extern CoinScoreUI* g_CoinUI;

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
static float ClampFloat(float v, float mn, float mx)
{
    if (v < mn) return mn;
    if (v > mx) return mx;
    return v;
}

static int ClampInt(int v, int mn, int mx)
{
    if (v < mn) return mn;
    if (v > mx) return mx;
    return v;
}

static float Rand01()
{
    return rand() / (float)RAND_MAX;
}

static float RandRange(float a, float b)
{
    return a + (b - a) * Rand01();
}

static XMFLOAT3 Lerp3(const XMFLOAT3& a, const XMFLOAT3& b, float t)
{
    XMFLOAT3 r;
    r.x = a.x + (b.x - a.x) * t;
    r.y = a.y + (b.y - a.y) * t;
    r.z = a.z + (b.z - a.z) * t;
    return r;
}

// ------------------------------------------------------------
// Cubic Bezier
// ------------------------------------------------------------
static XMFLOAT3 Bezier3_3(
    const XMFLOAT3& a,
    const XMFLOAT3& b,
    const XMFLOAT3& c,
    const XMFLOAT3& d,
    float t)
{
    float u = 1.0f - t;

    float uu = u * u;
    float tt = t * t;

    float w0 = uu * u;
    float w1 = 3.0f * uu * t;
    float w2 = 3.0f * u * tt;
    float w3 = tt * t;

    XMFLOAT3 r;

    r.x = a.x * w0 + b.x * w1 + c.x * w2 + d.x * w3;
    r.y = a.y * w0 + b.y * w1 + c.y * w2 + d.y * w3;
    r.z = a.z * w0 + b.z * w1 + c.z * w2 + d.z * w3;

    return r;
}

// ------------------------------------------------------------

void AirCurveChallenge::Initialize()
{
    SpawnCoinsToGoal();
}

void AirCurveChallenge::Reset()
{
    for (auto& c : g_Coins)
    {
        if (c.animPlayId >= 0)
            SpriteAnim_DestroyPlayer(c.animPlayId);
    }

    g_Coins.clear();
    SpawnCoinsToGoal();
}

// ------------------------------------------------------------
// MAIN SPAWN FUNCTION
// ------------------------------------------------------------
void AirCurveChallenge::SpawnCoinsToGoal()
{
    if (!g_CoinUI) return;

    //----------------------------------------------------------
    // Goal mouth center (based on CURRENT goal position)
    //----------------------------------------------------------
    XMFLOAT3 mouth;
    if (!GoalCollision_GetGoalMouthTarget(mouth))
        return;

    XMFLOAT3 target = mouth;

    //----------------------------------------------------------
    // Randomize inside goal
    //----------------------------------------------------------
    int roll = rand() % 100;

    if (roll < 45)
    {
        target.x += RandRange(-0.8f, 0.8f);
        target.y += RandRange(0.2f, 0.8f);
    }
    else if (roll < 80)
    {
        float side = (Rand01() < 0.5f) ? -1.0f : 1.0f;
        target.x += side * RandRange(1.2f, 2.0f);
        target.y += RandRange(0.4f, 1.2f);
    }
    else
    {
        float side = (Rand01() < 0.5f) ? -1.0f : 1.0f;
        target.x += side * RandRange(1.6f, 2.3f);
        target.y += RandRange(1.2f, 2.2f);
    }

    const float GOAL_HALF_WIDTH = 2.2f;
    const float GOAL_MIN_Y = 0.2f;
    const float GOAL_MAX_Y = 2.2f;

    // safety insets
    const float INSET_X = 0.30f;
    const float INSET_Y = 0.18f;

    target.z = mouth.z;

    target.x = ClampFloat(target.x,
        mouth.x - (GOAL_HALF_WIDTH - INSET_X),
        mouth.x + (GOAL_HALF_WIDTH - INSET_X));

    target.y = ClampFloat(target.y,
        mouth.y + (GOAL_MIN_Y + 0.05f),
        mouth.y + (GOAL_MAX_Y - INSET_Y));

    //----------------------------------------------------------
    // Lane type
    //----------------------------------------------------------
    enum LaneType
    {
        LANE_STRAIGHT,
        LANE_BANANA,
        LANE_LATE
    };

    LaneType lane;
    int laneRoll = rand() % 100;

    if (laneRoll < 35) lane = LANE_STRAIGHT;
    else if (laneRoll < 75) lane = LANE_BANANA;
    else lane = LANE_LATE;

    //----------------------------------------------------------
    // Start position = CURRENT BALL POSITION
    //----------------------------------------------------------
    XMFLOAT3 ballPos = BallPlayer_GetPosition();

    // start from the ball's current axis
    m_StartPoint = ballPos;

    // keep it slightly above ground for nice visible coin path
    float groundY = Mesh_GetHeightAt(m_StartPoint.x, m_StartPoint.z);
    m_StartPoint.y = groundY + 0.35f;

    // optional tiny push toward goal so first coin is not exactly inside the ball
    XMFLOAT3 toTarget =
    {
        target.x - m_StartPoint.x,
        0.0f,
        target.z - m_StartPoint.z
    };

    float len = sqrtf(toTarget.x * toTarget.x + toTarget.z * toTarget.z);
    if (len > 0.0001f)
    {
        toTarget.x /= len;
        toTarget.z /= len;

        m_StartPoint.x += toTarget.x * 1.0f;
        m_StartPoint.z += toTarget.z * 1.0f;
    } 
    //----------------------------------------------------------
    // Distance
    //----------------------------------------------------------
    float dx = target.x - m_StartPoint.x;
    float dy = target.y - m_StartPoint.y;
    float dz = target.z - m_StartPoint.z;

    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    int coinCount = ClampInt((int)(dist * 0.9f), 12, 26);
    float arcHeight = ClampFloat(dist * 0.12f, 2.4f, 5.0f);
    //----------------------------------------------------------
    // Direction vectors
    //----------------------------------------------------------
    XMVECTOR A = XMVectorSet(m_StartPoint.x, 0, m_StartPoint.z, 0);
    XMVECTOR D = XMVectorSet(target.x, 0, target.z, 0);

    XMVECTOR dir = XMVector3Normalize(D - A);
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, dir));

    float curveAmount = ClampFloat(dist * 0.36f, 5.0f, 13.0f);

    //----------------------------------------------------------
    // Control points
    //----------------------------------------------------------
    XMFLOAT3 B = Lerp3(m_StartPoint, target, 0.10f);
    XMFLOAT3 C = Lerp3(m_StartPoint, target, 0.90f);

    // lower curve body (NOT target)
    const float LOWER_Y = 0.9f;
    m_StartPoint.y -= LOWER_Y;
    B.y -= LOWER_Y;
    C.y -= LOWER_Y;

    //----------------------------------------------------------
    // Apply curve
    //----------------------------------------------------------
    if (lane != LANE_STRAIGHT)
    {
        float sign = (target.x > m_StartPoint.x) ? 1.0f : -1.0f;

        if (lane == LANE_BANANA)
        {
            float bAmt = curveAmount;
            float cAmt = curveAmount * 0.65f;

            B.x += XMVectorGetX(right) * bAmt * sign;
            B.z += XMVectorGetZ(right) * bAmt * sign;

            C.x += XMVectorGetX(right) * cAmt * sign;
            C.z += XMVectorGetZ(right) * cAmt * sign;
        }
        else // LANE_LATE
        {
            float bAmt = curveAmount * 0.2f;
            float cAmt = curveAmount * 1.1f;

            B.x += XMVectorGetX(right) * bAmt * sign;
            B.z += XMVectorGetZ(right) * bAmt * sign;

            C.x += XMVectorGetX(right) * cAmt * sign;
            C.z += XMVectorGetZ(right) * cAmt * sign;
        }
    }

    //----------------------------------------------------------
    // Spawn coins
    //----------------------------------------------------------
    for (int i = 0; i < coinCount; i++)
    {
        float t = (float)i / (coinCount - 1);

        XMFLOAT3 p = (lane == LANE_STRAIGHT)
            ? Lerp3(m_StartPoint, target, t)
            : Bezier3_3(m_StartPoint, B, C, target, t);

        float arc = sinf(t * XM_PI);
        arc = powf(arc, 0.65f);
        p.y += arc * arcHeight;

        Coin coin;
        coin.position = p;
        coin.spawnY = p.y;
        coin.collected = false;
        coin.timer = 0.0f;
        coin.animPlayId = SpriteAnim_CreatePlayer(g_CoinUI->GetCoinPattern());

        g_Coins.push_back(coin);
    }

    //----------------------------------------------------------
    // Force last coin exactly into goal
    //----------------------------------------------------------
    if (!g_Coins.empty())
    {
        g_Coins.back().position = target;
        g_Coins.back().spawnY = target.y;
    }
}