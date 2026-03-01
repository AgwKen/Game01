#pragma once
#include <DirectXMath.h>

class AirCurveChallenge
{
public:
    void Initialize();
    void Reset();

    DirectX::XMFLOAT3 GetStartPoint() const { return m_StartPoint; }

private:
    void SpawnCoinsToGoal();

private:
    DirectX::XMFLOAT3 m_StartPoint = { 0.0f, 1.0f, 3.0f };
};