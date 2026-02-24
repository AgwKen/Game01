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
extern DirectX::XMFLOAT3 g_GoalWorldOffset;
extern float g_GoalWorldBaseY;

bool GoalCollision_IsBallInsideGoal(const XMFLOAT3& ballPos, float radius);
bool GoalCollision_BackNetTouched();
void GoalCollision_ClearBackNetHit();


