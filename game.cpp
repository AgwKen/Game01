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
#include "shader_field.h"
#include "player_camera.h"
#include "map.h"
#include "bullet.h"
#include "billboard.h"
#include "sprite_anim.h"
#include "bullet_hit_effect.h"
#include "trajetory3d.h"
#include "sky.h"
#include "circle_shadow.h"
#include "Audio.h"
#include "shader3d_unlit.h"
#include "rendertextureclass.h"
#include "depthShader.h"
#include  "shadow.h"
#include "light_camera.h"
#include "coin.h"
#include "CoinScore.h"
#include "BallPlayer.h"
#include "particle_test.h"
#include "Goal.h"
#include "GoalCollision.h"
#include "AirCurveChallenge.h"
#include "sprite.h"

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

//testing pparticle effect 
static NormalEmitter* g_Emitter;

CoinScoreUI* g_CoinUI = nullptr;


static XMFLOAT3 g_TestLightPos = { 0.0f, 3.0f, 0.0f };

static float g_LightMoveTime = 0.0f;

static float g_DiscoSpeed = 3.0f;     // fast rotation
static float g_DiscoRadius = 4.0f;    // bigger circle

static XMFLOAT3 g_DiscoColor = { 1,1,1 };


static float g_AmbientLevel = 0.58f;   // default brightness

void Game_Initialize()
{

    // ==========================================
    // CREATE RENDER TEXTURE (IMPORTANT)
    // ==========================================
    g_pRenderTexture = new RenderTextureClass();

    g_pRenderTexture->Initialize(
        Direct3D_GetDevice(),
        Direct3D_GetBackBufferWidth(),
        Direct3D_GetBackBufferHeight(),
        1000.0f,   // screen depth
        0.1f,      // near plane
        1          // format
    );

InitAudio();
Camera_Initialize({ 8.2f, 8.4f, -12.7f }, { -0.5f, -0.3f, 0.7f }, { 0.8f, 0.0f, 0.5f });

// 3. Initialize the Shadow Shader (for drawing the final scene)
Shadow_Initialize();

g_pDepthShader = new DepthShaderClass();
g_pDepthShader->Initialize(Direct3D_GetDevice(), nullptr);

XMFLOAT3 center = { 0.0f, 0.0f, 0.0f }; // center of terrain

XMFLOAT3 lightDir = { 0.0f, 1.0f, 0.0f };

XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&lightDir));
XMStoreFloat3(&lightDir, dir);

// move light backward along direction
XMFLOAT3 lightPos = {
    center.x - lightDir.x * 80.0f,
    center.y - lightDir.y * 80.0f,
    center.z - lightDir.z * 80.0f
};

LightCamera_SetPosition(lightPos);
LightCamera_SetFront(lightDir);

Mesh_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

PadLogger_Initialize();
Bullet_Initialize();
Sky_Initialize();
PlayerCamera_Initialize();
Map_Initialize();
Billboard_Initialize();
BulletHitEffect_Initialize();
Trajetory3d_Initialize();
//Fog_Initialize();x
CircleShadow_Initialize();
BallPlayer_Initialize({ 0, 0, 0 }, 1.0f); // start position, radius
Goal_Initialize();

g_Emitter = new NormalEmitter(6000, { 0,0,0 }, 900.0, true);

g_BGM = LoadAudio("Sounds/bg.wav");
//PlayAudio(g_BGM, true);
//SetAudioVolume(g_BGM, 0.08f);

g_WindSE = LoadAudio("Sounds/wind.wav");
SetAudioVolume(g_WindSE, 0.15f);

// First wind after random delay (5?15 sec)
g_NextWindTime = 5.0f + (rand() % 10);


g_CoinUI = new CoinScoreUI(Direct3D_GetDevice(), Direct3D_GetDeviceContext(), 1280, 720);
g_CoinUI->SetCoinCount(g_PlayerCoinScore);

AirCurveChallenge::Initialize();

// ==============================
// TEST: Spawn one world coin
// ==============================

Coin testCoin;

testCoin.position = { 0.0f, 3.0f, 5.0f };  // in front of player
testCoin.spawnY = testCoin.position.y;
testCoin.collected = false;
testCoin.timer = 0.0f;

// IMPORTANT: use SAME pattern as UI
int coinPattern = g_CoinUI->GetCoinPattern();
testCoin.animPlayId = SpriteAnim_CreatePlayer(coinPattern);

g_Coins.push_back(testCoin);



}

void Game_Finalize()
{

    if (g_pDepthShader)
    {
        g_pDepthShader->Shutdown();
        delete g_pDepthShader;
        g_pDepthShader = nullptr;
    }

    if (g_pRenderTexture)
    {
        g_pRenderTexture->Shutdown();
        delete g_pRenderTexture;
        g_pRenderTexture = nullptr;
    }

	delete g_Emitter;
	g_Emitter = nullptr;
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
    // --- Now audio system shutdown ---
    UninitAudio();
    Goal_Finalize();
    BallPlayer_Finalize();

    // --- Rest ---
    CircleShadow_Finalize();
    Billboard_Finalize();
    Map_Finalize();
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

    // --- Dynamic Shadow Follow ---
    XMFLOAT3 lightDir = { 0.0f, 1.0f, 0.0f };  // angled sun, pointing DOWN

    XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&lightDir));
    XMStoreFloat3(&lightDir, dir);

    XMFLOAT3 focus = BallPlayer_GetPosition();

    XMFLOAT3 lightPos = {
        focus.x - lightDir.x * 80.0f,
        focus.y - lightDir.y * 80.0f,
        focus.z - lightDir.z * 80.0f
    };

    LightCamera_SetPosition(lightPos);
    LightCamera_SetFront(lightDir);


    // Increase ambient light with F2
    if (KeyLogger_IsTrigger(KK_F2))
    {
        g_AmbientLevel += 0.05f;
        if (g_AmbientLevel > 1.0f)
            g_AmbientLevel = 1.0f;
    }

    // Decrease ambient light with F3
    if (KeyLogger_IsTrigger(KK_F3))
    {
        g_AmbientLevel -= 0.05f;
        if (g_AmbientLevel < 0.0f)
            g_AmbientLevel = 0.0f;
    }


    if (KeyLogger_IsTrigger(KK_F1)) {
        g_IsDebug = !g_IsDebug;
    }

    g_AccumulatedTime += elapsed_time;


    PlayerCamera_Update(elapsed_time);

    BallPlayer_Update(elapsed_time);


    /*
    SpriteAnim_Update(elapsed_time);
    Bullet_Update(elapsed_time);
    BulletHitEffect_Update();
    Trajetory3d_Update(elapsed_time);
    */
    //can change debug Camera and player camera
    if (g_IsDebug) {
        Camera_Update(elapsed_time);
    }
    else {
        // Player_Update(elapsed_time);
    }

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

        XMFLOAT3 playerPos = BallPlayer_GetPosition();

        float collectDistance = 1.5f;

        float dx = coin.position.x - playerPos.x;
        float dy = coin.position.y - playerPos.y;
        float dz = coin.position.z - playerPos.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        if (coin.timer < 1.0f) {
            ++it;
            continue;
        }
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

    if (g_CoinUI)
        g_CoinUI->Update(elapsed_time);
    // --- PARTICLE FIX ---

    g_Emitter->Emmit(true);
    g_Emitter->Update(elapsed_time);


    // === DISCO LIGHT MOVEMENT ===
    g_LightMoveTime += (float)elapsed_time * g_DiscoSpeed;

    // Circle movement like disco ball
    g_TestLightPos.x = sinf(g_LightMoveTime) * g_DiscoRadius;
    g_TestLightPos.z = cosf(g_LightMoveTime) * g_DiscoRadius;
    g_TestLightPos.y = 3.0f + sinf(g_LightMoveTime * 2.0f); // small up/down bounce

    // Color cycling (RGB rainbow effect)
    g_DiscoColor.x = (sinf(g_LightMoveTime) + 1.0f) * 0.5f;
    g_DiscoColor.y = (sinf(g_LightMoveTime + 2.0f) + 1.0f) * 0.5f;
    g_DiscoColor.z = (sinf(g_LightMoveTime + 4.0f) + 1.0f) * 0.5f;

}
void RenderPass_Shadow()
{
    ID3D11DeviceContext* context = Direct3D_GetDeviceContext();

    Shadow_SetRenderTarget();
    Shadow_Clear();

    Direct3D_SetDepthEnable(true);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    XMMATRIX lightView = LightCamera_GetViewMatrix();
    XMMATRIX lightProj = LightCamera_GetProjectionMatrix();

    //  TERRAIN FIRST
    XMMATRIX terrainWorld = XMMatrixTranslation(0, 0, 0); // SAME AS Mesh_Draw
    Mesh_RenderDepth(g_pDepthShader, lightView, lightProj, terrainWorld);


    //  THEN BALL
    MODEL* model = BallPlayer_GetModel();
    if (model)
    {
        XMMATRIX world = BallPlayer_GetWorldMatrix();

        for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
        {
            UINT stride = 48;
            UINT offset = 0;

            context->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);
            context->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

            UINT indexCount = model->AiScene->mMeshes[m]->mNumFaces * 3;

            g_pDepthShader->Render(
                context,
                indexCount,
                world,
                lightView,
                lightProj,
                nullptr
            );
        }
    }


    XMMATRIX world = BallPlayer_GetWorldMatrix();
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)Direct3D_GetBackBufferWidth();
    vp.Height = (FLOAT)Direct3D_GetBackBufferHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

    context->RSSetViewports(1, &vp);

    Direct3D_SetRenderTarget();
}


void RenderPass_Offscreen()
{
    ID3D11DeviceContext* context = Direct3D_GetDeviceContext();

    g_pRenderTexture->SetRenderTarget(context);
    g_pRenderTexture->ClearRenderTarget(context, 0.1f, 0.1f, 0.1f, 1.0f);

    // --- CAMERA ---
    XMFLOAT4X4 mtxView = g_IsDebug ? Camera_GetMatrix() : PlayerCamera_GetViewMatrix();
    XMMATRIX view = XMLoadFloat4x4(&mtxView);
    XMMATRIX proj = g_IsDebug ?
        XMLoadFloat4x4(&Camera_GetPerspectiveMatrix()) :
        XMLoadFloat4x4(&PlayerCamera_GetPerspectiveMatrix());

    Camera_SetMatrix(view, proj);

    // --- SKY ---

    XMFLOAT3 camPos = g_IsDebug ? Camera_GetPosition() : PlayerCamera_GetPosition();
    Direct3D_SetDepthEnable(false);
    Sky_Draw(camPos);
    Direct3D_SetDepthEnable(true);


    // --- SHADOW MATRICES ---
    XMMATRIX lightView = LightCamera_GetViewMatrix();
    XMMATRIX lightProj = LightCamera_GetProjectionMatrix();

    ShaderField_SetShadowMatrices(lightView, lightProj);
    Shader3d_SetShadowMatrices(lightView, lightProj);

    ShaderField_SetViewMatrix(view);
    ShaderField_SetProjectionMatrix(proj);

    Shader3d_SetViewMatrix(view);
    Shader3d_SetProjectionMatrix(proj);

    //  BIND SHADOW MAP HERE
    ID3D11ShaderResourceView* shadowSRV = Shadow_GetShadowMap();
    context->PSSetShaderResources(2, 1, &shadowSRV);

    ID3D11SamplerState* shadowSampler = Shadow_GetSampler();
    context->PSSetSamplers(1, 1, &shadowSampler);

    // --- LIGHTING ---
    Light_SetAmbient({ 0.55f, 0.55f, 0.55f });

    ShaderField_SetAmbientColor({
        g_AmbientLevel,
        g_AmbientLevel,
        g_AmbientLevel,
        1.0f
    });

    XMVECTOR dirVec = XMVector3Normalize({ 0.6f, -1.0f, -0.4f });
    XMFLOAT4 dir;
    XMStoreFloat4(&dir, dirVec);

    XMFLOAT4 dirColor = { 0.85f, 0.85f, 0.85f, 1.0f };

    Light_SetDirectionalWorld(dir, dirColor);
    ShaderField_SetDirectionalLight(dir, dirColor);

    Light_SetSpecularWorld(
        g_IsDebug ? Camera_GetPosition() : PlayerCamera_GetPosition(),
        16.0f,
        { 1.0f, 1.0f, 1.0f, 1.0f }
    );

    // --- WORLD ---
    Map_Draw();
    BallPlayer_Draw();
    Goal_Render();

    // --- BILLBOARDS ---
    Billboard_SetViewMatrix(mtxView);

    for (auto& coin : g_Coins)
        Coin_Draw(coin);

    g_Emitter->Render();

 
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->PSSetShaderResources(2, 1, nullSRV);
}

void RenderPass_Main()
{
    Direct3D_SetRenderTarget();
    Direct3D_Clear();

    // DISABLE DEPTH HERE
    Direct3D_SetDepthEnable(false);

    ID3D11ShaderResourceView* srv = g_pRenderTexture->GetShaderResourceView();
    Direct3D_GetDeviceContext()->PSSetShaderResources(0, 1, &srv);

    Sprite_Begin();

    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    Sprite_DrawRaw(0, 0, screenW, screenH);

    // UNBIND
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    Direct3D_GetDeviceContext()->PSSetShaderResources(0, 1, nullSRV);

    // RE-ENABLE DEPTH
    Direct3D_SetDepthEnable(true);
}



void RenderPass_UI()
{
    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);

    if (g_CoinUI)
        g_CoinUI->Draw();


    Direct3D_SetDepthReadOnly(false);
    Direct3D_SetDefaultBlendState();
}


void Game_Draw()
{
    RenderPass_Shadow();
    RenderPass_Offscreen();   // Pass 1
    RenderPass_Main();        // Pass 2
    RenderPass_UI();          // UI
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