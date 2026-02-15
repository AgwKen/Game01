#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class AirCurveChallenge
{
public:
    static void Initialize();
    static void Reset();

private:
    static void SpawnCoins();
};
