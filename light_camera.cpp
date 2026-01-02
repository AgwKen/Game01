/*========================================================================================
	Light Camera [light_camera.h]                                   PYAE SONE THANT

																	DATE:26/11/2025

------------------------------------------------------------------------------------------
=========================================================================================*/
#include "light_camera.h"
using namespace DirectX;

static XMFLOAT3 g_Position{};
static XMFLOAT3 g_Front{ 0.0f, 1.0f, 0.0f };

void LightCamera_Initialize(const DirectX::XMFLOAT3& world_directional, const DirectX::XMFLOAT3& position)
{
	g_Front = world_directional;
	g_Position = position;
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
	return XMMatrixLookToLH(XMLoadFloat3(&g_Position), XMLoadFloat3(&g_Front), XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f });
}

DirectX::XMMATRIX LightCamera_GetProjectionMatrix()
{
	// Directional lights use Orthographic projection for shadows
	float size = 30.0f;
	return XMMatrixOrthographicLH(size, size, 0.1f, 1000.0f);
}
