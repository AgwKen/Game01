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
static int g_TexId = -1;
static int g_AnimPatternId = -1;
static int g_TreePlayerId = -1;
static XMFLOAT3 g_TreePosition = { -5.0f, 1.0f, 4.0f }; // Tree position

// --- Map objects ---
static MapObject g_MapObjects[]{
{FIELD, {0.0f, 0.0f, 0.0f}, {{-25.0f, -1.0f, -25.0f}, {25.0f, 0.0f, 25.0f}}},
{ HOUSE01, {0.0f, 1.0f, 0.0f}}
};

static MODEL* g_pModelHouse01{};

// --- Initialize ---
void Map_Initialize()
{
    // Load house model
    g_pModelHouse01 = ModelLoad("Resources/Model/house3.fbx", 0.005f);

    // Load tree texture
    g_TexId = Texture_Load(L"Texture/PineTree.png");

    // Register tree animation pattern (loop = true)
    g_AnimPatternId = SpriteAnim_RegisterPattern(
        g_TexId, 25, 5, 0.5, { 52, 80 }, { 0, 0 }, true);

    // Create tree animation player
    g_TreePlayerId = SpriteAnim_CreatePlayer(g_AnimPatternId);

	Mesh_SetCollisionParams(1.0f, 1.0f,-30.0f, -20.0f);

}

// --- Finalize ---
void Map_Finalize()
{
    ModelRelease(g_pModelHouse01);
}

// --- Update ---
void Map_Update(double elapsed_time)
{
    // Update the tree animation
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

}

// --- Helpers ---
int Map_GetObjectCount()
{
    return sizeof(g_MapObjects) / sizeof(g_MapObjects[0]);
}

const MapObject* Map_GetObject(int index)
{
    return &g_MapObjects[index];
}