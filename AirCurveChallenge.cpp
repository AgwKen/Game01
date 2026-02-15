#include "AirCurveChallenge.h"
#include "coin.h"
#include "sprite_anim.h"
#include "CoinScore.h"

extern std::vector<Coin> g_Coins;
extern CoinScoreUI* g_CoinUI;

void AirCurveChallenge::Initialize()
{
    SpawnCoins();
}

void AirCurveChallenge::Reset()
{
    g_Coins.clear();
    SpawnCoins();
}

void AirCurveChallenge::SpawnCoins()
{
    if (!g_CoinUI) return;

    int pattern = g_CoinUI->GetCoinPattern();

    g_Coins.clear();

    const int coinCount = 10;
    const float spacing = 2.0f;
    const float arcHeight = 4.0f;

    XMFLOAT3 start = { 0.0f, 1.0f, 3.0f };

    for (int i = 0; i < coinCount; i++)
    {
        float t = (float)i / (coinCount - 1);

        Coin coin;

        coin.position.x = start.x;
        coin.position.z = start.z + spacing * i;
        coin.position.y = start.y + sinf(t * XM_PI) * arcHeight;

        coin.spawnY = coin.position.y;
        coin.collected = false;
        coin.timer = 0.0f;

        coin.animPlayId = SpriteAnim_CreatePlayer(pattern);

        g_Coins.push_back(coin);
    }
}
