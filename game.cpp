/*========================================================================================

	Main Game [game.cpp]                                            PYAE SONE THANT
																	DATE:09/04/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "game.h"
#include "cube.h"
#include "shader3d.h"
#include "grid.h"
#include "Camera.h"
#include "Mesh.h"
#include "light.h"
#include "key_logger.h"
#include "direct3d.h"
#include <DirectXMath.h>
#include "model.h"
using namespace DirectX;


static float  g_angle = 0.0f;

static XMFLOAT3 g_CubePosition = {};
static XMFLOAT3 g_CubeVelocity = {};

static MODEL* g_pModelTest = nullptr;
//static MODEL* g_pModelCat = nullptr;


void Game_Initialize()
{
	Camera_Initialize({ 8.2f, 8.4f, -12.7f }, { -0.5f, -0.3f, 0.7f }, { 0.8f, 0.0f, 0.5f });
	Mesh_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

	g_pModelTest = ModelLoad("Resources/Model/test.fbx", 0.1f);
	//g_pModelCat = ModelLoad("Model/AlleyCat.fbx", 0.1f);

}

void Game_Finalize()
{
	ModelRelease(g_pModelTest);
	//ModelRelease(g_pModelCat);
	Mesh_Finalize();
	Camera_Finalize();
}

void Game_Update(double elapsed_time)
{
	g_angle += static_cast<float>(elapsed_time) * 3.0f;
	Camera_Update(elapsed_time);

	if (KeyLogger_IsTrigger(KK_SPACE)) {
		g_CubePosition = Camera_GetPosition();
		XMStoreFloat3(&g_CubeVelocity, XMLoadFloat3(&Camera_GetFront()) * 20.0f);
	}

	XMVECTOR cube_position = XMLoadFloat3(&g_CubePosition);
	XMVECTOR cube_velocity = XMLoadFloat3(&g_CubeVelocity);

	cube_position += cube_velocity * static_cast<float>(elapsed_time);
	XMStoreFloat3(&g_CubePosition, cube_position);
}

void Game_Draw()
{
	Light_SetAmbientColor({ 0.6f, 0.6f, 0.6f, 1.0f });
	Light_SetDirectionalWorld({ 1.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	Grid_Draw();
	XMMATRIX mtxWorld = XMMatrixTranslationFromVector(XMLoadFloat3(&g_CubePosition));
	Shader3d_SetWorldMatrix(mtxWorld);
	//CUBE_Draw(g_angle);
	//XMMATRIX mtxThrowCube = XMMatrixRotationY(g_angle * 0.5f);
	Mesh_Draw(1, 2, 0.0f, -30.0f, -20.0f);
	ModelDraw(g_pModelTest, XMMatrixTranslation(1.0f, 1.0f, 0.0f));
	//ModelDraw(g_pModelCat, XMMatrixTranslation(3.0f, 0.0f, 0.0f));

	Camera_DebugDraw();
}