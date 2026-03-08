#pragma once
#include <DirectXMath.h>

struct MODEL;

void Goal_Initialize();
void Goal_Finalize();

void Goal_Render();

bool Goal_HandleBallCollision(
    const DirectX::XMFLOAT3& prevBallPos,
    DirectX::XMFLOAT3& ballPos,
    DirectX::XMFLOAT3& ballVelocity,
    float ballRadius);

bool Goal_CheckScored(const DirectX::XMFLOAT3& pos, float r);

// NEW
void Goal_SetWorldPosition(float x, float z);
void Goal_RandomizePlacement();

DirectX::XMMATRIX Goal_GetWorldMatrix();
MODEL* Goal_GetModel();

void Goal_SetWorldOffset(const DirectX::XMFLOAT3& offset);
DirectX::XMFLOAT3 Goal_GetWorldPosition();