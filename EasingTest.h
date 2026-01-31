#pragma once
#include <DirectXMath.h>

class EasingCube
{
private:
	double m_AccumulatedTime{};
	double m_Duration{ 2.0 };
	DirectX::XMFLOAT3 m_StartPos{};
	DirectX::XMFLOAT3 m_EndPos{};
	bool m_IsStart{};

public:
	EasingCube(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end,double time)
		: m_StartPos(start), m_EndPos(end), m_Duration(time)
	{

	}
	void Start(){ m_IsStart = true; };
	void Update(double elapsed_time);
	void Draw() const;
};