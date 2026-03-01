#include "AirCurveChallenge.h"

#include "coin.h"
#include "sprite_anim.h"
#include "CoinScore.h"
#include "GoalCollision.h"
#include "terrain.h"     // Mesh_GetHeightAt

#include <vector>
#include <cmath>

using namespace DirectX;

extern std::vector<Coin> g_Coins;
extern CoinScoreUI* g_CoinUI;

// ------------------------------------------------------------
// Manual clamp helpers (NO std::clamp)
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

static DirectX::XMFLOAT3 Lerp3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t)
{
    DirectX::XMFLOAT3 out;
    out.x = a.x + (b.x - a.x) * t;
    out.y = a.y + (b.y - a.y) * t;
    out.z = a.z + (b.z - a.z) * t;
    return out;
}

void AirCurveChallenge::Initialize()
{
    SpawnCoinsToGoal();
}

void AirCurveChallenge::Reset()
{
    g_Coins.clear();
    SpawnCoinsToGoal();
}

void AirCurveChallenge::SpawnCoinsToGoal()
{
    if (!g_CoinUI) return;

    // ------------------------------------------------------------
    // Goal mouth target (center inside posts)
    // ------------------------------------------------------------
    XMFLOAT3 target;
    if (!GoalCollision_GetGoalMouthTarget(target))
        return;

    // ------------------------------------------------------------
// Balanced MID/FAR system (no extreme values)
// ------------------------------------------------------------
    int roll = rand() % 100;

    float minForward = 16.0f;
    float maxForward = 20.0f;  // default mid

    if (roll < 25)
    {
        // 25%: slightly far
        minForward = 20.0f;
        maxForward = 24.0f;
    }
    else if (roll < 80)
    {
        // 55%: mid (most common)
        minForward = 16.0f;
        maxForward = 20.0f;
    }
    else
    {
        // 20%: slightly closer (but not close)
        minForward = 14.0f;
        maxForward = 16.0f;
    }

    float forward = minForward + (maxForward - minForward) * Rand01();

    // Side range increases with distance (far shots can be angled more)
    float sideRange = 3.0f + forward * 0.12f;
    float side = (Rand01() * 2.0f - 1.0f) * sideRange;

    // Start point: in front of goal (toward player side) => target.z - forward
    m_StartPoint.x = target.x + side;
    m_StartPoint.z = target.z - forward;

    // Snap to ground (so player runs to a real area, not floating)
    float groundY = Mesh_GetHeightAt(m_StartPoint.x, m_StartPoint.z);
    m_StartPoint.y = groundY + 1.0f; // coins lane height ~1m above ground

    // ------------------------------------------------------------
    // Coin settings derived from distance
    // ------------------------------------------------------------
    int pattern = g_CoinUI->GetCoinPattern();

    float dx = target.x - m_StartPoint.x;
    float dy = target.y - m_StartPoint.y;
    float dz = target.z - m_StartPoint.z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    // More distance => more coins (feels like a "lane")
    int coinCount = (int)(dist * 0.75f);
    coinCount = ClampInt(coinCount, 10, 22);

    // Arc height scales with distance
    float arcHeight = dist * 0.18f;
    arcHeight = ClampFloat(arcHeight, 3.0f, 10.0f);

    // ------------------------------------------------------------
    // Spawn coins along arc from START -> GOAL
    // ------------------------------------------------------------
    for (int i = 0; i < coinCount; i++)
    {
        float t = (coinCount <= 1) ? 0.0f : (float)i / (float)(coinCount - 1);

        XMFLOAT3 p = Lerp3(m_StartPoint, target, t);

        // Arc hump (highest in middle)
        p.y += sinf(t * XM_PI) * arcHeight;

        Coin coin;
        coin.position = p;
        coin.spawnY = p.y;
        coin.collected = false;
        coin.timer = 0.0f;
        coin.animPlayId = SpriteAnim_CreatePlayer(pattern);

        g_Coins.push_back(coin);
    }
}