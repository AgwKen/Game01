#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct BallState
{
    XMFLOAT3 position;
    XMFLOAT3 prevPosition;
    XMFLOAT3 velocity;
    XMVECTOR angularVelocity;

    bool onGround = false;
};
