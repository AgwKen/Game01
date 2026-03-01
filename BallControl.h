#pragma once
#include <DirectXMath.h>
#include "BallState.h"

// This struct is exactly what BallPlayer needs when a kick is released.
struct BallKickRelease
{
    bool released = false;          // became true on the frame we released charge
    bool doCinematic = false;       // charge01 >= KICK_CINEMATIC_MIN_CHARGE01

    DirectX::XMFLOAT3 dir = { 0,0,1 };
    float power = 0.0f;
    float lift = 0.0f;
    float curve = 0.0f;             // already includes aimInput.x scaling (same as your code)
};

// Initialize / reset
void BallControl_Reset();

// Update aim input (arrow keys + right stick). Call only when NOT kicked.
void BallControl_UpdateAimInput();

// Dribble movement (same math you had): modifies ball velocity XZ directly
void BallControl_UpdateDribble(float dt, BallState& ball, bool isKicked, bool hitStopActive);

// Kick charge logic (space / trigger). Returns release info when charge is released.
BallKickRelease BallControl_UpdateKickCharge(float dt, bool isKicked, bool hitStopActive);

// Air curve control (WASD / left stick) for AIR_CURVE_CONTROL_DURATION after kick
void BallControl_UpdateAirCurveControl(float dt, BallState& ball, bool isKicked);

// Helpers / getters
DirectX::XMFLOAT3 BallControl_BuildAimDirection();
DirectX::XMFLOAT3 BallControl_GetAimDirection(bool isKicked);

float BallControl_GetKickCharge();
bool  BallControl_IsCharging();
float BallControl_GetKickMaxPower();

// Needed because BallPlayer_Kick starts the control timer (same behavior as before)
void BallControl_OnKickApplied();
