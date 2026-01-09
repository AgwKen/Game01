#pragma once
#include <DirectXMath.h>
#include <vector>
using namespace DirectX;

struct Coin
{
    XMFLOAT3 position;
    int animPlayId;
    bool collected = false;
    float timer = 0.0f;      // bounce animation
    float spawnY = 0.0f;
    float collectTimer = 0.0f; // NEW: counts time after collection
};


void Coin_Update(Coin& coin, double elapsed);
void Coin_Draw(const Coin& coin);

// Global coin array and score
extern std::vector<Coin> g_Coins;
extern int g_PlayerCoinScore;
