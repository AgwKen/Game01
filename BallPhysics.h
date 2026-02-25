#pragma once
#include "BallState.h"

void BallPhysics_Initialize(BallState& state, const DirectX::XMFLOAT3& startPos);
void BallPhysics_Update(BallState& state, float dt);