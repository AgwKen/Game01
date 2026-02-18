/*========================================================================================
	Light Camera [light_camera.h]                                   PYAE SONE THANT

																	DATE:26/11/2025

------------------------------------------------------------------------------------------
=========================================================================================*/
#include "light_camera.h"
using namespace DirectX;

static XMFLOAT3 g_Position{};
static XMFLOAT3 g_Front{ 0.0f, 1.0f, 0.0f };

void LightCamera_Initialize(const XMFLOAT3& world_directional, const XMFLOAT3& position)
{
	g_Position = position;

	XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&world_directional));
	XMStoreFloat3(&g_Front, dir);
}

void LightCamera_Finalize()
{
}

void LightCamera_SetPosition(const DirectX::XMFLOAT3& position)
{
	g_Position = position;
}

void LightCamera_SetFront(const DirectX::XMFLOAT3& front)
{
	g_Front = front;
}

DirectX::XMMATRIX LightCamera_GetViewMatrix()
{
	// Use your existing position and front direction
	XMVECTOR up;

	if (fabs(g_Front.y) > 0.99f)
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);   // use Z axis as up
	else
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	return XMMatrixLookToLH(
		XMLoadFloat3(&g_Position),
		XMLoadFloat3(&g_Front),
		up
	);

}

DirectX::XMMATRIX LightCamera_GetProjectionMatrix()
{
	float size = 40.0f;   // must cover 0-100 terrain
	float nearZ = 1.0f;
	float farZ = 200.0f;

	return XMMatrixOrthographicLH(size, size, nearZ, farZ);

}


DirectX::XMFLOAT3 LightCamera_GetPosition()
{
	return g_Position;
}