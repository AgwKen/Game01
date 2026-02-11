#pragma once
#include <DirectXMath.h>
#include "collision.h"

using namespace DirectX;

// Initialize / Finalize
void Goal_Initialize();
void Goal_Finalize();

// Draw model and debug
void Goal_Draw();

// Main collision with ball
bool Goal_HandleBallCollision(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius
);

// Optional scoring detection later
bool Goal_CheckScored(const XMFLOAT3& ballPos, float ballRadius);
