/*========================================================================================

    Main Game [game.cpp]                                           PYAE SONE THANT
                                                                    DATE:09/04/2025
------------------------------------------------------------------------------------------

=========================================================================================*/
#include "game.h"
#include "shader3d.h"
#include "grid.h"
#include "Camera.h"
#include "Mesh.h"
#include "light.h"
#include "key_logger.h"
#include "direct3d.h"
#include <DirectXMath.h>
#include "model.h"
#include "sampler.h"
using namespace DirectX;
#include "player.h"
#include "shader_field.h"
#include "player_camera.h"
#include "cube.h"
#include "map.h"

static float  g_angle = 0.0f;
static XMFLOAT3 g_CubePosition = {};
static XMFLOAT3 g_CubeVelocity = {};
static double g_AccumulatedTime = 0.0;
static MODEL* g_pModelTest = nullptr;
static MODEL* g_pModelTest2 = nullptr;
static MODEL* g_pModelTest3 = nullptr;



void Game_Initialize()
{
    Camera_Initialize({ 8.2f, 8.4f, -12.7f }, { -0.5f, -0.3f, 0.7f }, { 0.8f, 0.0f, 0.5f });
    Mesh_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    Player_Initialize({ 0.0f, 1.0f, -5.0f }, { 0.0f, 0.0f, 1.0f });
    PlayerCamera_Initialize();

	Map_Initialize();

    g_pModelTest = ModelLoad("Resources/Model/tree.fbx", 0.1f);
    g_pModelTest2 = ModelLoad("Resources/Model/Formula 1 mesh.fbx", 0.01f);
    g_pModelTest3 = ModelLoad("Resources/Model/snowman.fbx", 0.001f);
}

void Game_Finalize()
{
	Map_Finalize();
    Player_Finalize();
    Mesh_Finalize();
    PlayerCamera_Finalize();
    Camera_Finalize();
}

// game.cpp (in Game_Update)

void Game_Update(double elapsed_time)
{
    g_AccumulatedTime += elapsed_time;

    // --- ADDED: Check for toggle key (F1) ---
    if (KeyLogger_IsTrigger(KK_F1))
    {
        PlayerCamera_ToggleMode();
    }
    // ----------------------------------------

    PlayerCamera_Update(elapsed_time);

    // --- MODIFIED: Conditional Player Update ---
    if (PlayerCamera_GetMode() == CameraMode::PLAYER_FOLLOW)
    {
        Player_Update(elapsed_time);
    }
    // --------------------------------------------

    if (PlayerCamera_GetMode() == CameraMode::DEBUG_FREE)
    {
        Camera_Update(elapsed_time); // Call Camera_Update ONLY in debug mode
    }
}
void Game_Draw()
{
    //===== Ambient Light =====//
    Light_SetAmbient({ 0.3f, 0.3f, 0.3f });
    ShaderField_SetAmbientColor({ 0.3f, 0.3f, 0.3f, 1.0f });

    //===== Directional Light =====//
    XMVECTOR v = XMVector3Normalize({ -1.0f, -1.0f, 1.0f });
    XMFLOAT4 dir;
    XMStoreFloat4(&dir, v);
    XMFLOAT4 dirColor = { 0.3f, 0.25f, 0.2f, 1.0f };
    Light_SetDirectionalWorld(dir, dirColor);
    ShaderField_SetDirectionalLight(dir, dirColor);

    //===== Grid =====//
    Grid_Draw();

    //===== Point Lights =====//
    Light_SetPointLightCount(0);

    XMMATRIX rot = XMMatrixRotationY(g_angle);
    XMFLOAT3 pp0, pp1, pp2;
    XMStoreFloat3(&pp0, XMVector3Transform({ 0.0f, 2.0f, -3.0f }, rot));
    XMStoreFloat3(&pp1, XMVector3Transform({ 0.0f, 3.0f,  3.0f }, rot));
    XMStoreFloat3(&pp2, XMVector3Transform({ 0.0f, 4.0f,  2.0f }, rot));

    Light_SetPointLight(0, pp0, 5.0f, { 1.0f, 0.0f, 0.0f });
    Light_SetPointLight(1, pp1, 3.0f, { 0.0f, 1.0f, 0.0f });
    Light_SetPointLight(2, pp2, 3.0f, { 0.0f, 0.0f, 1.0f });

    //===== Specular Light =====//
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 50.0f, { 0.3f, 0.3f, 0.3f, 0.3f });
    Light_SetSpecularWorld(Camera_GetPosition(), 1.0f, { 0.1f, 0.1f, 0.1f, 1.0f });

    //===== Draw Mesh =====//
    Mesh_Draw(1, 2, 0.0f, -30.0f, -20.0f);

    //===== Draw Cube =====//
    //Sampler_SetFilterAnisotropic();
    //XMMATRIX mtxCube = XMMatrixTranslation(3.0f, 0.5f, 2.0f);
    //CUBE_Draw(mtxCube);

	Map_Draw();


    //===== Draw Player =====//
    Player_Draw();

    ModelDraw(g_pModelTest, XMMatrixTranslation(2.0f, 0.1f, 0.0f));
    ModelDraw(g_pModelTest2, XMMatrixTranslation(-6.0f, 0.1f, 0.0f));
    ModelDraw(g_pModelTest3, XMMatrixTranslation(6.0f, 0.4f, 0.0f));
    
    //===== Debug =====//
    if (PlayerCamera_GetMode() == CameraMode::DEBUG_FREE)
    {
        Camera_DebugDraw();
    }
}
