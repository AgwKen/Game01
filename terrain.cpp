/*========================================================================================

    Mesh cpp [Mesh.cpp]									    	        PYAE SONE THANT
                                                                        DATE:09/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "terrain.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader_field.h"
#include "texture.h"
#include "camera.h"
#include <cmath>
#include <cstdlib>

/*========================================================================================
    Constants
=========================================================================================*/
static constexpr float FIELD_MESH_SIZE = 1.0f;
static constexpr int FIELD_MESH_H_COUNT = 200;
static constexpr int FIELD_MESH_V_COUNT = 200;
static constexpr int FIELD_MESH_H_VERTEX_COUNT = FIELD_MESH_H_COUNT + 1;
static constexpr int FIELD_MESH_V_VERTEX_COUNT = FIELD_MESH_V_COUNT + 1;
static constexpr int NUM_VERTEX = FIELD_MESH_H_VERTEX_COUNT * FIELD_MESH_V_VERTEX_COUNT;
static constexpr int NUM_INDEX = 3 * 2 * FIELD_MESH_H_COUNT * FIELD_MESH_V_COUNT;

/*========================================================================================
    D3D Resources
=========================================================================================*/
static ID3D11Buffer* g_pVertexBuffer = nullptr;
static ID3D11Buffer* g_pIndexBuffer = nullptr;

static int g_Tex0Id = -1;// grass
static int g_Tex1Id = -1;//moutain rock

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


static float g_CollisionOffsetX = 0.0f;
static float g_CollisionOffsetZ = 0.0f;
static int g_RepeatX = 1;
static int g_RepeatZ = 1;

struct Vertex3d
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT4 color;
    XMFLOAT2 texcoord;
};

static Vertex3d g_MeshVertex[NUM_VERTEX];
static unsigned short g_MeshIndex[NUM_INDEX];

enum class TerrainShapeType
{
    Cone,
    Plateau,
    Valley,
    SmoothHill
};

struct TerrainShape
{
    TerrainShapeType type;
    float x;
    float z;
    float radius;
    float height;
    float extra;   // edge falloff, etc.
};

/* === DATA ONLY (easy to edit) === */
static TerrainShape g_TerrainShapes[] =
{
    // Mountains
    { TerrainShapeType::SmoothHill,    40.0f,  25.0f, 10.0f,  3.0f, 0.0f },
    { TerrainShapeType::Cone,    70.0f,  10.0f, 10.0f,  5.0f, 0.0f },

    // Mount Everest
    { TerrainShapeType::Cone,   100.0f, 100.0f, 30.0f, 18.0f, 0.0f },

    // Plateau
    { TerrainShapeType::Plateau, 50.0f,  80.0f, 20.0f,  5.0f, 5.0f },

    // Valley
    { TerrainShapeType::Valley, 120.0f, 200.0f, 15.0f,  4.0f, 0.0f },
};

static int g_NumShapes = sizeof(g_TerrainShapes) / sizeof(TerrainShape);

static float EvaluateShapeHeight(const TerrainShape& shape, float worldX, float worldZ)
{
    float dx = worldX - shape.x;
    float dz = worldZ - shape.z;
    float dist = sqrtf(dx * dx + dz * dz);

    switch (shape.type)
    {
    case TerrainShapeType::Cone:
        if (dist < shape.radius)
            return shape.height * (1.0f - dist / shape.radius);
        break;

    case TerrainShapeType::Plateau:
        if (dist < shape.radius)
            return shape.height;
        else if (dist < shape.radius + shape.extra)
            return shape.height * (1.0f - (dist - shape.radius) / shape.extra);
        break;

    case TerrainShapeType::Valley:
        if (dist < shape.radius)
            return -shape.height * (1.0f - dist / shape.radius);
        break;

    case TerrainShapeType::SmoothHill:
        if (dist < shape.radius)
        {
            float t = dist / shape.radius;
            return shape.height * expf(-t * t * 3.0f);
        }
        break;
    }

    return 0.0f;
}


void Mesh_SetCollisionParams(int repeatX, int repeatZ, float offsetX, float offsetZ)
{
    g_RepeatX = repeatX;
    g_RepeatZ = repeatZ;
    g_CollisionOffsetX = offsetX;
    g_CollisionOffsetZ = offsetZ;
}

void Mesh_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;

    for (int z = 0; z < FIELD_MESH_V_VERTEX_COUNT; ++z)
    {
        for (int x = 0; x < FIELD_MESH_H_VERTEX_COUNT; ++x)
        {
            int index = x + z * FIELD_MESH_H_VERTEX_COUNT;
            float worldX = x * FIELD_MESH_SIZE;
            float worldZ = z * FIELD_MESH_SIZE;

            float height = 0.0f;

            // === Terrain Shapes ===
            for (int i = 0; i < g_NumShapes; ++i)
            {
                height += EvaluateShapeHeight(g_TerrainShapes[i], worldX, worldZ);
            }

            // === Random Noise ===
            float noise = ((rand() % 20) / 100.0f - 0.5f) * 0.5f;
            height += noise;

            g_MeshVertex[index].position = { worldX, height, worldZ };
            g_MeshVertex[index].normal = { 0.0f, 1.0f, 0.0f };
            g_MeshVertex[index].color = { 0.0f, 1.0f, 1.0f, 1.0f };
            g_MeshVertex[index].texcoord = { float(x), float(z) };
        }
    }

    int k = 0;
    for (int z = 0; z < FIELD_MESH_V_COUNT; ++z)
    {
        for (int x = 0; x < FIELD_MESH_H_COUNT; ++x)
        {
            int tl = x + z * FIELD_MESH_H_VERTEX_COUNT;
            int tr = (x + 1) + z * FIELD_MESH_H_VERTEX_COUNT;
            int bl = x + (z + 1) * FIELD_MESH_H_VERTEX_COUNT;
            int br = (x + 1) + (z + 1) * FIELD_MESH_H_VERTEX_COUNT;

            g_MeshIndex[k++] = tl;
            g_MeshIndex[k++] = bl;
            g_MeshIndex[k++] = tr;
            g_MeshIndex[k++] = bl;
            g_MeshIndex[k++] = br;
            g_MeshIndex[k++] = tr;
        }
    }

    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = g_MeshVertex;
    g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

    bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    sd.pSysMem = g_MeshIndex;
    g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);

    g_Tex0Id = Texture_Load(L"Texture/grass.jpg");
    g_Tex1Id = Texture_Load(L"Texture/rock.jpg");
    ShaderField_Initialize(g_pDevice, g_pContext);
}

void Mesh_Finalize()
{
    ShaderField_Finalize();
    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pIndexBuffer);
}

void Mesh_Draw(int repeatX, int repeatZ, float heightOffset, float offsetX, float offsetZ)
{
    ShaderField_Begin();
    Texture_SetTexture(g_Tex0Id, 0); //grass
    Texture_SetTexture(g_Tex1Id, 1); // moutain

    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;

    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ShaderField_SetWorldMatrix(XMMatrixTranslation(offsetX, heightOffset, offsetZ));
    g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}

float Mesh_GetHeightAt(float worldX, float worldZ)
{
    // Convert world Å® terrain local
    float localX = worldX - g_CollisionOffsetX;
    float localZ = worldZ - g_CollisionOffsetZ;

    float height = 0.0f;

    for (int i = 0; i < g_NumShapes; ++i)
    {
        height += EvaluateShapeHeight(g_TerrainShapes[i], localX, localZ);
    }

    return height;
}
