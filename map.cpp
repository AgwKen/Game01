#include "map.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "cube.h"
#include "texture.h"
#include "light.h"
#include "mesh.h"
#include "player_camera.h"
#include "camera.h"
#include "model.h"
#include "direct3d.h"
#include "sprite_anim.h"
#include "billboard.h"
#include "sampler.h"

// --- Tree animation ---
static int g_TreeTexId = -1;
static int g_Tree_animPatternId = -1;
static int g_TreePlayerId = -1;
static XMFLOAT3 g_TreePosition = { -5.0f, 0.9f, 9.0f }; // Tree position

static int g_Tree2TexId = -1;
static int g_Tree2_animPatternId = -1;
static int g_Tree2PlayerId = -1;
static XMFLOAT3 g_Tree2Position = { -3.0f, 0.9f, 8.0f }; // Tree position


// --- Map objects ---
static MapObject g_MapObjects[]{
{FIELD, {0.0f, 0.0f, 0.0f}, {{-25.0f, -1.0f, -25.0f}, {25.0f, 0.0f, 25.0f}}},
{ HOUSE01, {0.0f, 1.0f, 0.0f}}
};

static MODEL* g_pModelHouse01{};

// --- Initialize ---
void Map_Initialize()
{

    g_TreeTexId = Texture_Load(L"Texture/PineTree.png");
    g_Tree_animPatternId = SpriteAnim_RegisterPattern(
    g_TreeTexId, 25, 5, 0.2, { 52, 80 }, { 0, 0 }, true);
    g_TreePlayerId = SpriteAnim_CreatePlayer(g_Tree_animPatternId);

    g_Tree2TexId = Texture_Load(L"Texture/Tree2.png");
    g_Tree2_animPatternId = SpriteAnim_RegisterPattern(
    g_Tree2TexId, 25, 5, 0.2, { 77, 130 }, { 0, 0 }, true);
    g_Tree2PlayerId = SpriteAnim_CreatePlayer(g_Tree2_animPatternId);

    //house
    g_pModelHouse01 = ModelLoad("Resources/Model/house3.fbx", 0.005f);

    Mesh_SetCollisionParams(1.0f, 1.0f, -30.0f, -20.0f);

}

void Map_Finalize()
{
    Texture_Release(g_Tree2TexId);
    Texture_Release(g_TreeTexId);
    ModelRelease(g_pModelHouse01);
}

void Map_Update(double elapsed_time)
{
    SpriteAnim_Update(elapsed_time);
}

// --- Draw ---
void Map_Draw()
{
    XMMATRIX mtxWorld;

    for (const MapObject& obj : g_MapObjects)
    {
        switch (obj.KindId)
        {
        case FIELD:
            Light_SetSpecularWorld(PlayerCamera_GetPosition(), 50.0f, { 0.3f,0.3f,0.3f,0.3f });
            Light_SetSpecularWorld(Camera_GetPosition(), 1.0f, { 0.1f,0.1f,0.1f,1.0f });
            Mesh_Draw(1, 2, 0.0f, -30.0f, -20.0f);
            break;

        case HOUSE01:
            mtxWorld = XMMatrixTranslation(obj.Position.x, obj.Position.y, obj.Position.z);
            ModelDraw(g_pModelHouse01, mtxWorld);
            break;
        }
    }

    for (int i = 0; i < 10; i++)
    {
        // Offset each tree along X axis (straight line)
        XMFLOAT3 pos;
        pos.x = g_TreePosition.x + (float)i * 3.0f; // spacing = 3 units
        pos.y = g_TreePosition.y;
        pos.z = g_TreePosition.z;

        // Set alpha blending + depth test, no depth write
        Direct3D_SetAlphaBlendState();
        Direct3D_SetDepthReadOnly(true);
        Sampler_SetFilterPoint();

        BillboardAnim_Draw(g_TreePlayerId, pos, { 3.0f, 3.0f }, { 0.5f, 1.0f });

        // Restore states for next objects
        Direct3D_SetDefaultBlendState();
        Direct3D_SetDepthEnable(true);
    }

    for (int i = 0; i < 10; i++)
    {
        XMFLOAT3 pos2;
        pos2.x = g_Tree2Position.x + (float)i * 3.0f;
        pos2.y = g_Tree2Position.y;
        pos2.z = g_Tree2Position.z;

        // Set alpha blending + depth test, no depth write
        Direct3D_SetAlphaBlendState();
        Direct3D_SetDepthReadOnly(true);
        Sampler_SetFilterPoint();

        BillboardAnim_Draw(g_Tree2PlayerId, pos2, { 3.0f, 3.0f }, { 0.5f, 1.0f });

        // Restore states for next objects
        Direct3D_SetDefaultBlendState();
        Direct3D_SetDepthEnable(true);
    }

}

int Map_GetObjectCount()
{
    return sizeof(g_MapObjects) / sizeof(g_MapObjects[0]);
}

const MapObject* Map_GetObject(int index)
{
    return &g_MapObjects[index];
}