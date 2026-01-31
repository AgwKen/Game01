#include "EasingTest.h"
#include "cube.h"
using namespace DirectX;
#include <algorithm>

void EasingCube::Update(double elapsed_time)
{
	if (!m_IsStart) return;

	m_AccumulatedTime += elapsed_time;
}

void EasingCube::Draw() const
{
	XMVECTOR start = XMLoadFloat3(&m_StartPos);
	XMVECTOR end = XMLoadFloat3(&m_EndPos);
	XMVECTOR v = end - start;

	float ratio = static_cast<float>(std::min(m_AccumulatedTime / m_Duration,1.0));

	v *= ratio;
	v += start;
	
	XMMATRIX world = XMMatrixTranslationFromVector(v);

	CUBE_Draw(2, world);
}
