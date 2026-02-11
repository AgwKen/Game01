#pragma once
#include <DirectXMath.h>

using namespace DirectX;

void Goal_Initialize();
void Goal_Finalize();

void Goal_Render();

bool Goal_HandleBallCollision(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius
);

bool Goal_CheckScored(const XMFLOAT3& ballPos, float ballRadius);
