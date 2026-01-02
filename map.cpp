/*===============================================================================

  Map cpp[Map.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/11/11
---------------------------------------------------------------------------------

=================================================================================*/
#include "map.h"
#include <DirectXMath.h>
using namespace DirectX;
#include <vector>
#include <cstdlib>
#include "cube.h"
#include "texture.h"
#include "light.h"
#include "terrain.h"
#include "player_camera.h"
#include "camera.h"
#include "model.h"
#include "direct3d.h"
#include "sprite_anim.h"
#include "billboard.h"
#include "sampler.h"


static float RandRange(float min, float max)
{
    return min + (float)rand() / RAND_MAX * (max - min);
}

static int g_TreeTexId = -1;
static int g_Tree_animPatternId = -1;
static int g_TreePlayerId = -1;
static XMFLOAT3 g_TreePosition = { -28.0f, 0.9f, 9.0f };

static int g_Tree2TexId = -1;
static int g_Tree2_animPatternId = -1;
static int g_Tree2PlayerId = -1;
static XMFLOAT3 g_Tree2Position = { -30.0f, 0.9f, 7.0f };

static int g_GrassTexId = -1;

struct GrassInstance
{
    XMFLOAT3 pos;
    XMFLOAT2 size;
};

static std::vector<GrassInstance> g_GrassInstances;

static MapObject g_MapObjects[]{
    { FIELD,   {0.0f, 0.0f, 0.0f}, {{-25.0f, -1.0f, -25.0f}, {25.0f, 0.0f, 25.0f}} },
    { HOUSE01, {-4.0f, 1.0f, 0.0f} }
};

static MODEL* g_pModelHouse01 = nullptr;

void AddGrassCircle(const XMFLOAT3& center, float radius, float spacing)
{
    for (float x = -radius; x <= radius; x += spacing)
    {
        for (float z = -radius; z <= radius; z += spacing)
        {
            if ((x * x + z * z) > radius * radius)
                continue;

            GrassInstance g;
            g.pos = {
                center.x + x + RandRange(-0.15f, 0.15f),
                center.y + RandRange(-0.03f, 0.02f),
                center.z + z + RandRange(-0.15f, 0.15f)
            };

            g.size = {
                RandRange(0.18f, 0.26f), // width
                RandRange(0.22f, 0.35f)  // height
            };

            g_GrassInstances.push_back(g);
        }
    }
}


void Map_Initialize()
{
    // Trees
    g_TreeTexId = Texture_Load(L"Texture/PineTree.png");
    g_Tree_animPatternId = SpriteAnim_RegisterPattern(
        g_TreeTexId, 25, 5, 0.2f, { 52, 80 }, { 0, 0 }, true);
    g_TreePlayerId = SpriteAnim_CreatePlayer(g_Tree_animPatternId);

    g_Tree2TexId = Texture_Load(L"Texture/Tree2.png");
    g_Tree2_animPatternId = SpriteAnim_RegisterPattern(
        g_Tree2TexId, 25, 5, 0.2f, { 77, 130 }, { 0, 0 }, true);
    g_Tree2PlayerId = SpriteAnim_CreatePlayer(g_Tree2_animPatternId);

    // House
    g_pModelHouse01 = ModelLoad("Resources/Model/house3.fbx", 0.005f);

    // Grass
    g_GrassTexId = Texture_Load(L"Texture/TallGrass.png");

    // Collision
    Mesh_SetCollisionParams(1, 2, -30.0f, -20.0f);

    srand(1); // fixed seed for stable layout

    XMFLOAT3 grassCenter = { -10.0f, 0.05f, -5.0f };
    float radius = 6.0f;
    float grassSpacing = 0.4f;

    g_GrassInstances.clear();

    // Add several grass circles
    AddGrassCircle({ -10.0f, 0.05f, -5.0f }, 6.0f, 0.9f);
    AddGrassCircle({ 0.0f, 0.05f, -10.0f }, 3.0f, 0.3f);
    AddGrassCircle({ 54.0f, 18.0f, 15.0f }, 11.0f, 0.5f);


    for (float x = -radius; x <= radius; x += grassSpacing)
    {
        for (float z = -radius; z <= radius; z += grassSpacing)
        {
            if ((x * x + z * z) > radius * radius)
                continue;

            GrassInstance g;

            g.pos = {
                grassCenter.x + x + RandRange(-0.15f, 0.15f),
                grassCenter.y + RandRange(-0.03f, 0.02f),
                grassCenter.z + z + RandRange(-0.15f, 0.15f)
            };

            g.size = {
                RandRange(0.18f, 0.26f),//width
                RandRange(0.22f, 0.35f),//height
            };



            g_GrassInstances.push_back(g);
        }
    }
}

void Map_Finalize()
{
    Texture_Release(g_GrassTexId);
    Texture_Release(g_Tree2TexId);
    Texture_Release(g_TreeTexId);

    ModelRelease(g_pModelHouse01);

    g_GrassInstances.clear();
}

void Map_Update(double elapsed_time)
{
    SpriteAnim_Update(g_TreePlayerId);
    SpriteAnim_Update(g_Tree2PlayerId);
}

void Map_Draw()
{
    XMMATRIX mtxWorld;

    // Map objects
    for (const MapObject& obj : g_MapObjects)
    {
        switch (obj.KindId)
        {
        case FIELD:
            Light_SetSpecularWorld(PlayerCamera_GetPosition(), 50.0f,
                { 0.3f, 0.3f, 0.3f, 0.3f });
            Light_SetSpecularWorld(Camera_GetPosition(), 1.0f,
                { 0.1f, 0.1f, 0.1f, 1.0f });
            Mesh_Draw(1, 2, 0, -30, -20);
            break;

        case HOUSE01:
            mtxWorld = XMMatrixTranslation(
                obj.Position.x,
                obj.Position.y,
                obj.Position.z);
            ModelDraw(g_pModelHouse01, mtxWorld);
            break;
        }
    }

 // for trees
    int treeCountX = 10;
    int treeCountZ = 5;
    float spacing = 5.0f;

    for (int x = 0; x < treeCountX; x++)
    {
        for (int z = 0; z < treeCountZ; z++)
        {
            XMFLOAT3 pos = {
                g_TreePosition.x + x * spacing,
                g_TreePosition.y,
                g_TreePosition.z + z * spacing
            };

            Sampler_SetFilterPoint();
            BillboardAnim_Draw(g_TreePlayerId, pos, { 5.0f, 5.0f }, { 0.5f, 1.5f });
        }
    }

    for (int x = 0; x < treeCountX; x++)
    {
        for (int z = 0; z < treeCountZ; z++)
        {
            XMFLOAT3 pos2 = {
                g_Tree2Position.x + x * 4.0f,
                g_Tree2Position.y,
                g_Tree2Position.z + z * 4.0f
            };

            Sampler_SetFilterPoint();
            BillboardAnim_Draw(g_Tree2PlayerId, pos2, { 5.0f, 5.0f }, { 0.5f, 1.5f });
        }
    }
    
    Billboard_SetViewMatrix(PlayerCamera_GetViewMatrix());
    Sampler_SetFilterPoint();

    for (const GrassInstance& g : g_GrassInstances)
    {
        Billboard_Draw(
            g_GrassTexId,
            g.pos,
            g.size,
            XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
            { 0.5f, 0.0f },
            { 1, 1, 1, 1 }
        );
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

