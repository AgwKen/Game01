/*========================================================================================


    Main Game [game.h]									    			PYAE SONE THANT
                                                                        DATE:06/27/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "game.h"
#include "shader3d.h"
#include "grid.h"
#include "Camera.h"
#include "Mesh.h"
#include "light.h"
#include "key_logger.h"
#include "pad_logger.h"
#include "direct3d.h"
#include <DirectXMath.h>
#include "model.h"
#include "sampler.h"
#include "player.h"
#include "shader_field.h"
#include "player_camera.h"
#include "map.h"
#include "bullet.h"
#include "billboard.h"
#include "sprite_anim.h"
#include "bullet_hit_effect.h"
#include "trajetory3d.h"
#include "sky.h"
#include "enemy.h"


using namespace DirectX;

static float g_angle = 0.0f;
static double g_AccumulatedTime = 0.0;
static bool g_IsDebug = false;

void Game_Initialize()
{
Camera_Initialize({ 8.2f, 8.4f, -12.7f }, { -0.5f, -0.3f, 0.7f }, { 0.8f, 0.0f, 0.5f });
Mesh_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
Player_Initialize({ 0.0f, 0.0f, -5.0f }, { 0.0f, 0.0f, 1.0f });
PadLogger_Initialize();
Enemy_Initialize();
Bullet_Initialize();
Sky_Initialize();
PlayerCamera_Initialize();
Map_Initialize();
Billboard_Initialize();
BulletHitEffect_Initialize();
Trajetory3d_Initialize();

Enemy_Create({5.0f,1.0f,5.0f});

}

void Game_Finalize()
{
BulletHitEffect_Finalize();
Billboard_Finalize();
Map_Finalize();
Enemy_Finalize();
Player_Finalize();
Sky_Finalize();
Bullet_Finalize();
Mesh_Finalize();
PlayerCamera_Finalize();
Camera_Finalize();
Trajetory3d_Finalize();
}

void Game_Update(double elapsed_time)
{
    PadLogger_Update();

    if (KeyLogger_IsTrigger(KK_F1)) {
        g_IsDebug = !g_IsDebug;
    }

    g_AccumulatedTime += elapsed_time;

    SpriteAnim_Update(elapsed_time);
    PlayerCamera_Update(elapsed_time);
    Bullet_Update(elapsed_time);
    BulletHitEffect_Update();
    Trajetory3d_Update(elapsed_time);

    //can change debug Camera and player camera
    if (g_IsDebug) {
        Camera_Update(elapsed_time);
    }
    else {
        Player_Update(elapsed_time);
    }

    Enemy_Update(elapsed_time);

    Sky_SetPosition(Player_GetPosition());


    //Bullet Collision
    for (int j = 0; j < Map_GetObjectCount(); j++) {
        for (int i = 0; i < Bullet_GetBulletsCount(); i++) {
            AABB bullet = Bullet_GetAABB(i);
            AABB object = Map_GetObject(j)->Aabb;

            if (Collision_IsOverlapAABB(bullet, object)) {
                BulletHitEffect_Create(Bullet_GetPosition(i));
                Bullet_Destroy(i);
            }
        }
    }
    
    for (int j = 0; j < Enemy_GetEnemyCount(); j++) {
        for (int i = 0; i < Bullet_GetBulletsCount(); i++) {
            Sphere bullet = Bullet_GetSphere(i);
            Sphere enemy = Enemy_GetEnemy(j)->GetCollision();
            if (Collision_IsOverlapSphere(bullet, enemy)) {
                BulletHitEffect_Create(Bullet_GetPosition(i));
                Bullet_Destroy(i);
                Enemy_GetEnemy(j)->Damage(50);
            }
        }
    }
	
}

void Game_Draw()
{
// --- CAMERA ---
XMFLOAT4X4 mtxView = g_IsDebug ? Camera_GetMatrix() : PlayerCamera_GetViewMatrix();
XMMATRIX view = XMLoadFloat4x4(&mtxView);
XMMATRIX proj = g_IsDebug ?
XMLoadFloat4x4(&Camera_GetPerspectiveMatrix()) :
XMLoadFloat4x4(&PlayerCamera_GetPerspectiveMatrix());
Camera_SetMatrix(view, proj);

XMFLOAT3 camera_position = g_IsDebug ? Camera_GetPosition() : PlayerCamera_GetPosition();
Billboard_SetViewMatrix(mtxView);
Sampler_SetFilterAnisotropic();

Sky_Draw();

// Ambient Light
Light_SetAmbient({ 0.3f, 0.3f, 0.3f });
ShaderField_SetAmbientColor({ 0.8f, 0.8f, 0.8f, 1.0f });

// Directional Light
XMVECTOR dirVec = XMVector3Normalize({ -1.0f, -1.0f, 1.0f });
XMFLOAT4 dir;
XMStoreFloat4(&dir, dirVec);
XMFLOAT4 dirColor = { 1.0f, 1.0f, 1.0f, 1.0f };
Light_SetDirectionalWorld(dir, dirColor);
ShaderField_SetDirectionalLight(dir, dirColor);

// Specular Light
Light_SetSpecularWorld(Camera_GetPosition(), 1.0f, {0.1f, 0.1f, 0.1f, 1.0f});

// Point Lights
Light_SetPointLightCount(0);
//========================

//--- GRID ---
Grid_Draw();


//--- DRAW OBJECTS ---[
Map_Draw();
Bullet_Draw();

Enemy_Draw();

Player_Draw();

//--- BULLET HIT EFFECTS ---
Direct3D_SetAlphaBlendState();
Direct3D_SetDepthReadOnly(true);
BulletHitEffect_Draw();
Direct3D_SetDepthReadOnly(false);
Direct3D_SetDefaultBlendState();

Direct3D_SetAlphaBlendState();
Direct3D_SetDepthReadOnly(true);
Direct3D_SetDepthReadOnly(false);
Direct3D_SetDefaultBlendState();

// Inside Game_Draw()
Direct3D_SetSubtractiveBlendState();  // or Additive
Direct3D_SetDepthReadOnly(true);      // so particles donÅft write to depth
Trajetory3d_Draw();
Direct3D_SetDepthReadOnly(false);
Direct3D_SetDefaultBlendState();

//--- DEBUG ---
if (g_IsDebug) {
    Camera_DebugDraw();
}

}
