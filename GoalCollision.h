#pragma once

#include <vector>
#include <DirectXMath.h>
#include "model.h"

using namespace DirectX;

void GoalCollision_Initialize();
void GoalCollision_Finalize();

bool GoalCollision_HandleBall(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius
);

void GoalCollision_GetDebugBoxes(std::vector<AABB>& out);

void GoalCollision_SetOffset(const DirectX::XMFLOAT3& pos);
extern DirectX::XMFLOAT3 g_GoalWorldOffset;
extern float g_GoalWorldBaseY;

