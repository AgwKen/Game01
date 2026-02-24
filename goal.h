#pragma once
#include <DirectXMath.h>
#include "model.h"


using namespace DirectX;

void Goal_Initialize();
void Goal_Finalize();

void Goal_Render();

bool Goal_HandleBallCollision(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius
);

bool Goal_CheckScored(const XMFLOAT3& pos, float r);
XMMATRIX Goal_GetWorldMatrix();
MODEL* Goal_GetModel();
void Goal_SetWorldOffset(const XMFLOAT3& offset);
XMFLOAT3 Goal_GetWorldPosition();