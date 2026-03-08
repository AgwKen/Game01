#ifndef GOAL_KEEPER_H
#define GOAL_KEEPER_H

#include <DirectXMath.h>

void GoalKeeper_Initialize();
void GoalKeeper_Finalize();
void GoalKeeper_Update(double dt);
void GoalKeeper_Render();

// ball collision
bool GoalKeeper_HandleBallCollision(
    DirectX::XMFLOAT3& ballPos,
    DirectX::XMFLOAT3& ballVelocity,
    float ballRadius);

// helpers
DirectX::XMFLOAT3 GoalKeeper_GetPosition();

#endif