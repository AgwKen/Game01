#pragma once
#include <DirectXMath.h>

using namespace DirectX;

// -----------------------------------------------------------------------------
// Ball Player (Rolling Ball)
// -----------------------------------------------------------------------------
void BallPlayer_Kick(
    const XMFLOAT3& kickDir,
    float power,
    float lift,
    float curve
);
void BallPlayer_Initialize(const XMFLOAT3& startPos, float radius);
void BallPlayer_Update(double elapsedTime);
void BallPlayer_Draw();
void BallPlayer_Finalize();

// --- Utility ---
XMFLOAT3 BallPlayer_GetPosition();
void BallPlayer_SetPosition(const XMFLOAT3& pos);

