#include "coin.h"
#include "sprite_anim.h"
#include "billboard.h"
#include <vector>
#include <cmath> // for sinf

// Global coin array
std::vector<Coin> g_Coins;

// Player's coin score
int g_PlayerCoinScore = 0;

void Coin_Update(Coin& coin, double elapsed)
{
    if (coin.collected) return; // skip bounce/animation
    if (coin.animPlayId < 0) return;

    SpriteAnim_UpdatePlayer(coin.animPlayId, elapsed);

    coin.timer += (float)elapsed;
    coin.position.y = coin.spawnY + sinf(coin.timer * 5.0f) * 0.2f;
}

void Coin_Draw(const Coin& coin)
{
    if (coin.collected) return;
    if (coin.animPlayId < 0) return;

    BillboardAnim_Draw(
        coin.animPlayId,
        coin.position,
        { 0.5f, 0.5f },  // scale
        { 0.5f, 0.5f }   // pivot
    );
}
