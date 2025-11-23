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

// --- Tree animation ---
static int g_TexId = -1;
static int g_AnimPatternId = -1;
static int g_TreePlayerId = -1;
static XMFLOAT3 g_TreePosition = { -5.0f, 1.0f, 0.0f }; // Tree position

// --- Map objects ---
static MapObject g_MapObjects[]{
{FIELD, {0.0f, 0.0f, 0.0f}, {{-25.0f, -1.0f, -25.0f}, {25.0f, 0.0f, 25.0f}}},
{HOUSE01, {0.0f, 1.5f, 0.0f}}
};

static MODEL* g_pModelHouse01{};

// --- Initialize ---
void Map_Initialize()
{
    // Load house model
    g_pModelHouse01 = ModelLoad("Resources/Model/house3.fbx", 0.007f);

    // Load tree texture
    g_TexId = Texture_Load(L"Texture/PineTree.png");

    // Register tree animation pattern (loop = true)
    g_AnimPatternId = SpriteAnim_RegisterPattern(
        g_TexId, 25, 5, 0.5, { 52, 80 }, { 0, 0 }, true);

    // Create tree animation player
    g_TreePlayerId = SpriteAnim_CreatePlayer(g_AnimPatternId);

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

    Direct3D_SetAlphaBlendState(); // enable alpha blending
    BillboardAnim_Draw(g_TreePlayerId, g_TreePosition, { 2.5f, 2.5f }, { 0.5f, 1.0f });
    Direct3D_SetDefaultBlendState(); // restore default


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