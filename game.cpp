/*========================================================================================


    Main Game [game.cpp]									    			PYAE SONE THANT
                                                                        DATE:06/27/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "game.h"
#include "shader3d.h"
#include "grid.h"
#include "Camera.h"
#include "terrain.h"
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
//#include "fog.h"
#include "circle_shadow.h"
#include "Audio.h"
#include "shader3d_unlit.h"
#include "rendertextureclass.h"
#include "depthShader.h"
#include  "shadow.h"
#include "light_camera.h"
#include "coin.h"
#include "CoinScore.h"


static float g_angle = 0.0f;
static double g_AccumulatedTime = 0.0;
static bool g_IsDebug = false;
static float g_HitStopTime = 0.0f;

//Background music
static int g_BGM = -1;

// Wind ambience
static int g_WindSE = -1;
static float g_WindTimer = 0.0f;
static float g_NextWindTime = 0.0f;
static float bgmDelayTimer = 0.0f;
static bool bgmStarted = false;

static RenderTextureClass* g_pRenderTexture = nullptr;
static DepthShaderClass* g_pDepthShader = nullptr;

//coin
static CoinScoreUI* g_CoinUI = nullptr;


void Game_Initialize()
{
InitAudio();
Camera_Initialize({ 8.2f, 8.4f, -12.7f }, { -0.5f, -0.3f, 0.7f }, { 0.8f, 0.0f, 0.5f });

// 3. Initialize the Shadow Shader (for drawing the final scene)
Shadow_Initialize();

// 4. Initialize Light Camera (Position the "Sun")
// Looking from (10, 20, 10) down towards the center (0,0,0)
XMFLOAT3 lightPos = { 10.0f, 20.0f, 10.0f };
XMFLOAT3 lightDir = { -0.5f, -1.0f, -0.5f };
LightCamera_Initialize(lightDir, lightPos);

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
//Fog_Initialize();
CircleShadow_Initialize();

g_BGM = LoadAudio("Sounds/bg.wav");
//PlayAudio(g_BGM, true);
//SetAudioVolume(g_BGM, 0.08f);

g_WindSE = LoadAudio("Sounds/wind.wav");
SetAudioVolume(g_WindSE, 0.15f);

// First wind after random delay (5?15 sec)
g_NextWindTime = 5.0f + (rand() % 10);


Enemy_Create({6.0f,5.0f,0.0f});

g_CoinUI = new CoinScoreUI(Direct3D_GetDevice(), Direct3D_GetDeviceContext(), 1280, 720);
g_CoinUI->SetCoinCount(g_PlayerCoinScore);



}

void Game_Finalize()
{
    // --- Stop & unload BGM ---
    if (g_BGM >= 0)
    {
        StopAudio(g_BGM);
        UnloadAudio(g_BGM);
        g_BGM = -1;
    }

    // --- Stop & unload wind ---
    if (g_WindSE >= 0)
    {
        StopAudio(g_WindSE);
        UnloadAudio(g_WindSE);
        g_WindSE = -1;
    }

    // --- Player unloads its own sounds ---
    Player_Finalize();

    // --- Now audio system shutdown ---
    UninitAudio();

    // --- Rest ---
    CircleShadow_Finalize();
    Billboard_Finalize();
    Map_Finalize();
    Enemy_Finalize();
    Sky_Finalize();
    Bullet_Finalize();
    Mesh_Finalize();
    PlayerCamera_Finalize();
    Camera_Finalize();
    Trajetory3d_Finalize();
    delete g_CoinUI;
    g_CoinUI = nullptr;

}

void Game_Update(double elapsed_time)
{
    PadLogger_Update();

    /*
   if (!bgmStarted)
   {
       bgmDelayTimer += (float)elapsed_time;
       if (bgmDelayTimer >= 7.0f)
       {
           PlayAudio(g_BGM, true);
           SetAudioVolume(g_BGM, 0.08f);
           bgmStarted = true;
       }
   }
   */


    if (KeyLogger_IsTrigger(KK_F1)) {
        g_IsDebug = !g_IsDebug;
    }

    // === HIT STOP ===
    if (g_HitStopTime > 0.0f)
    {
        g_HitStopTime -= (float)elapsed_time;
        if (g_HitStopTime < 0.0f)
            g_HitStopTime = 0.0f;

        SpriteAnim_Update(elapsed_time * 0.2f);
        return;
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

    //Fog_Update(elapsed_time);


    //Bullet Collision
    /*
    for (int j = 0; j < Map_GetObjectCount(); j++)
    {
        auto* obj = Map_GetObject(j);
        if (!obj) continue;

        for (int i = Bullet_GetBulletsCount() - 1; i >= 0; i--)
        {
            AABB bullet = Bullet_GetAABB(i);
            AABB object = obj->Aabb;

            if (Collision_IsOverlapAABB(bullet, object))
            {
                BulletHitEffect_Create(Bullet_GetPosition(i));
                Bullet_Destroy(i);
            }
        }
    }

    for (int j = 0; j < Enemy_GetEnemyCount(); j++) {
        for (int i = Bullet_GetBulletsCount() - 1; i >= 0; i--) {
            Sphere bullet = Bullet_GetSphere(i);
            Sphere enemy = Enemy_GetEnemy(j)->GetCollision();

            if (Collision_IsOverlapSphere(bullet, enemy)) {
                BulletHitEffect_Create(Bullet_GetPosition(i));
                Bullet_Destroy(i);
                Enemy_GetEnemy(j)->Damage(50);
            }
        }
    }
    */
    /*
    // TEST SPAWN FOG EVERY SECOND
    static float fogTimer = 0;
    fogTimer += (float)elapsed_time;

    if (fogTimer > 1.0f)
    {
        XMFLOAT3 pos = Player_GetPosition();
        pos.x += (rand() % 100 - 50) * 0.1f;
        pos.z += (rand() % 100 - 50) * 0.1f;
        pos.y += 0.5f;

        Fog_Spawn(pos, 2.5f, 3.0f);
        fogTimer = 0;
    }
    */
    // === WIND AMBIENCE ===
    g_WindTimer += (float)elapsed_time;

    if (g_WindTimer >= g_NextWindTime)
    {
        PlayAudio(g_WindSE, false); // play once (NOT loop)

        g_WindTimer = 0.0f;

        // Next wind between 8?20 seconds
        g_NextWindTime = 8.0f + (rand() % 12);
    }
    // Inside Game_Update in game.cpp

    for (auto it = g_Coins.begin(); it != g_Coins.end(); )
    {
        Coin& coin = *it;
        Coin_Update(coin, elapsed_time);

        XMFLOAT3 playerPos = Player_GetPosition();

        // 1. INCREASE DISTANCE: 1.0f might be too small for the visual size. 
        // Try 1.5f or 2.0f for a better "snag" feel.
        float collectDistance = 1.5f;

        float dx = coin.position.x - playerPos.x;
        float dy = coin.position.y - playerPos.y;
        float dz = coin.position.z - playerPos.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (!coin.collected && distSq < (collectDistance * collectDistance))
        {
            // 2. IMMEDIATE FEEDBACK: Update score and UI
            g_PlayerCoinScore += 1;
            if (g_CoinUI) {
                g_CoinUI->SetCoinCount(g_PlayerCoinScore);
            }

            // 3. REMOVAL: Since you aren't playing a "collection animation," 
            // just erase it immediately to make it feel snappy.
            it = g_Coins.erase(it);
            continue;
        }
        ++it;
    }

    // DELETE THIS BLOCK BELOW in your game.cpp:
    // This block is redundant and causing confusion because 'collected' 
    // is never set to true in your current logic.
    /*
    g_Coins.erase(
        std::remove_if(g_Coins.begin(), g_Coins.end(),
            [](const Coin& c) { return c.collected && c.collectTimer > 0.01f; }),
        g_Coins.end()
    );
    */
    if (g_CoinUI)
        g_CoinUI->Update(elapsed_time);

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

    Billboard_SetViewMatrix(mtxView);
    Shader3dUnlit_Begin();

    // --- SKY ---
    XMFLOAT3 camPos = g_IsDebug
        ? Camera_GetPosition()
        : PlayerCamera_GetPosition();

    Sky_Draw(camPos);

    // --- LIGHTING ---
    Light_SetAmbient({ 0.15f, 0.15f, 0.15f });
    ShaderField_SetAmbientColor({ 0.08f, 0.08f, 0.08f, 1.0f });

    XMVECTOR dirVec = XMVector3Normalize({ 0.6f, -1.0f, -0.4f });
    XMFLOAT4 dir;
    XMStoreFloat4(&dir, dirVec);

    XMFLOAT4 dirColor = { 0.85f, 0.85f, 0.85f, 1.0f }; // not pure white

    Light_SetDirectionalWorld(dir, dirColor);
    ShaderField_SetDirectionalLight(dir, dirColor);


    Light_SetSpecularWorld(Camera_GetPosition(), 1.0f, { 0.1f, 0.1f, 0.1f, 1.0f });
    Light_SetPointLightCount(0);

    // --- DRAW SCENE ---
    Grid_Draw();
    Map_Draw();
    Bullet_Draw();
    Enemy_Draw();
    Player_Draw();
    // --- Draw coins ---
    for (auto& coin : g_Coins)
    {
        BillboardAnim_Draw(coin.animPlayId, coin.position, { 0.15f, 0.15f }, { 0.5f, 1.0f });
    }


    // --- EFFECTS ---
    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    //Fog_Draw();
    Direct3D_SetDepthReadOnly(false);
    Direct3D_SetDefaultBlendState();

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    BulletHitEffect_Draw();
    Direct3D_SetDepthReadOnly(false);
    Direct3D_SetDefaultBlendState();

    Direct3D_SetSubtractiveBlendState();
    Direct3D_SetDepthReadOnly(true);
    Trajetory3d_Draw();
    Direct3D_SetDepthReadOnly(false);
    Direct3D_SetDefaultBlendState();

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    if (g_CoinUI)
        g_CoinUI->Draw();
    Direct3D_SetDepthReadOnly(false);
    Direct3D_SetDefaultBlendState();


    // --- DEBUG ---
    if (g_IsDebug) {
        Camera_DebugDraw();
    }
}


bool Game_IsHitStopActive()
{
    return g_HitStopTime > 0.0f;
}

void Game_RequestHitStop(float time)
{
    if (time > g_HitStopTime)
        g_HitStopTime = time;
}