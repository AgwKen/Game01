/*========================================================================================

    terrain cpp [Mesh.cpp]                                                PYAE SONE THANT
                                                                        DATE:09/11/2025

-----------------------------------------------------------------------------------------

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
#include <algorithm>
#include "shadow.h"
#include "depthShader.h"


static constexpr float FIELD_MESH_SIZE = 1.0f;
static constexpr int FIELD_MESH_H_COUNT = 100;
static constexpr int FIELD_MESH_V_COUNT = 100;
static constexpr int FIELD_MESH_H_VERTEX_COUNT = FIELD_MESH_H_COUNT + 1;
static constexpr int FIELD_MESH_V_VERTEX_COUNT = FIELD_MESH_V_COUNT + 1;
static constexpr int NUM_VERTEX = FIELD_MESH_H_VERTEX_COUNT * FIELD_MESH_V_VERTEX_COUNT;
static constexpr int NUM_INDEX = 3 * 2 * FIELD_MESH_H_COUNT * FIELD_MESH_V_COUNT;

static ID3D11Buffer* g_pVertexBuffer = nullptr;
static ID3D11Buffer* g_pIndexBuffer = nullptr;

static int g_Tex0Id = -1; // grass
static int g_Tex1Id = -1; // rock

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
    SmoothHill,
    FlatSurfaceMoutain,
    slopeMountain,
    GradualHill,
    Ridge,
    RidgeHorizontal
};

struct TerrainShape
{
    TerrainShapeType type;
    float x;
    float z;
    float radius;
    float height;
    float extra; // length
};

static TerrainShape g_TerrainShapes[] =
{
    { TerrainShapeType::SmoothHill,    40.0f,  25.0f, 10.0f,  2.0f, 0.0f },
    { TerrainShapeType::SmoothHill,    15.0f,  100.0f, 50.0f,  20.0f, 0.0f },
  //  { TerrainShapeType::Cone,          70.0f,  10.0f, 10.0f,  5.0f, 0.0f },
   // { TerrainShapeType::FlatSurfaceMoutain, 120.0f,  90.0f, 35.0f, 14.0f, 0.0f },
   // { TerrainShapeType::slopeMountain,80.0f, 40.0f, 40.0f, 18.0f, 0.0f },
    { TerrainShapeType::GradualHill, 100.0f, 50.0f, 60.0f, 5.0f, 0.0f },// very gradual, low height
   // { TerrainShapeType::Plateau,       50.0f,  80.0f, 20.0f,  5.0f, 5.0f },
    //{ TerrainShapeType::Valley,       120.0f, 200.0f, 15.0f,  4.0f, 0.0f },
     { TerrainShapeType::GradualHill, 28.0f, 5.0f, 60.0f, 5.0f, 0.0f },
    // { TerrainShapeType::Plateau,       30.0f,  80.0f, 20.0f,  5.0f, 5.0f },
     { TerrainShapeType::Plateau,       40.0f,  70.0f, 10.0f,  5.0f, 5.0f },
     { TerrainShapeType::Ridge,     0.0f,  90.0f, 10.0f,  10.0f, 120.0f },
     { TerrainShapeType::Ridge,     100.0f,  90.0f, 10.0f,  10.0f, 120.0f },
   { TerrainShapeType::RidgeHorizontal,     60.0f,  100.0f, 10.0f,  10.0f, 40.0f },
    { TerrainShapeType::Plateau,       50.0f,  80.0f, 1.0f,  10.0f, 50.0f },
    //  { TerrainShapeType::RidgeHorizontal,     50.0f,  0.0f, 10.0f,  10.0f, 100.0f },
   //{ TerrainShapeType::RidgeHorizontal,     2.0f,  90.0f, 10.0f,  10.0f, 120.0f },
};


static int g_NumShapes = sizeof(g_TerrainShapes) / sizeof(TerrainShape);

static float EvaluateShapeHeight(const TerrainShape& shape, float worldX, float worldZ)
{
    float dx = worldX - shape.x;
    float dz = worldZ - shape.z;

    float distort = sinf(worldX * 0.12f) * 2.0f + cosf(worldZ * 0.15f) * 2.5f;
    float dist = sqrtf(dx * dx + dz * dz) + distort;

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
    case TerrainShapeType::FlatSurfaceMoutain:
        if (dist < shape.radius)
        {
            float baseRadius = shape.radius * 0.5f;  // start slope from 0 to 50% radius
            if (dist < baseRadius)
            {
                float t = dist / baseRadius;  // 0 at center, 1 at end of base slope
                return shape.height * powf(t, 2.0f); // gentle rise at base
            }

            // Flat top region
            float edge = shape.radius * 0.85f;  // top flat until 85% of radius
            if (dist < edge)
                return shape.height;

            // Steep cliff at the edges
            float t = (dist - edge) / (shape.radius - edge);
            t = std::min(t, 1.0f);
            return shape.height * (1.0f - t * t * t * 4.0f);
        }
        break;

    case TerrainShapeType::slopeMountain:
        if (dist < shape.radius)
        {
            float t = dist / shape.radius;  // 0 at center, 1 at edge
            t = std::min(t, 1.0f);

            float plateauRatio = 0.4f; // top 20% is flat
            float ascentCurve = 1.0f;  // smaller = more gradual ascent

            if (t < plateauRatio)
            {
                return shape.height;
            }
            else
            {
                // Smooth slope from base to plateau
                float slopeT = (t - plateauRatio) / (1.0f - plateauRatio); // normalize to 0..1
                return shape.height * (1.0f - powf(slopeT, ascentCurve));
            }
        }
        break;
    case TerrainShapeType::Ridge:
    {
        float nx = fabsf(dx) / shape.radius;  // width
        float nz = fabsf(dz) / shape.extra;   // length

        float dist = sqrtf(nx * nx + nz * nz);
        if (dist >= 1.0f)
            break;

        float core = 0.35f;
        if (dist < core)
            return shape.height;

        float t = (dist - core) / (1.0f - core);
        return shape.height * (1.0f - t * t);
    }
    break;
    case TerrainShapeType::RidgeHorizontal:
    {
        float nx = fabsf(dx) / shape.extra;   // length
        float nz = fabsf(dz) / shape.radius;  // width

        float dist = sqrtf(nx * nx + nz * nz);
        if (dist >= 1.0f)
            break;

        float core = 0.35f;
        if (dist < core)
            return shape.height;

        float t = (dist - core) / (1.0f - core);
        return shape.height * (1.0f - t * t);
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

float Mesh_GetVertexNoise(float x, float z)
{
    // deterministic pseudo-random function based on position
    return (sinf(x * 0.3f + z * 0.7f) * 0.15f + cosf(x * 0.5f - z * 0.2f) * 0.15f) * 0.5f;
}


float Mesh_GetHeightAt(float worldX, float worldZ)
{
    // Apply collision offsets
    float localX = worldX - g_CollisionOffsetX;
    float localZ = worldZ - g_CollisionOffsetZ;

    // Sum the procedural shapes for collision
    float height = 0.0f;
    for (int i = 0; i < g_NumShapes; ++i)
    {
        height += EvaluateShapeHeight(g_TerrainShapes[i], localX, localZ);
    }

    // If you want, you can add tiny deterministic noise that doesn't break slopes
    // e.g., a simple sine-based bump:
  // add deterministic noise that matches mesh
    height += Mesh_GetVertexNoise(localX, localZ);

    return height;
}

void Mesh_RenderDepth(
    DepthShaderClass* depthShader,
    DirectX::XMMATRIX lightView,
    DirectX::XMMATRIX lightProj,
    DirectX::XMMATRIX worldMatrix)
{
    if (!depthShader) return;

    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;

    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DirectX::XMMATRIX world = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    depthShader->Render(
        g_pContext,
        NUM_INDEX,
        worldMatrix,
        lightView,
        lightProj,
        nullptr
    );

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
            for (int i = 0; i < g_NumShapes; ++i)
                height += EvaluateShapeHeight(g_TerrainShapes[i], worldX, worldZ);

            float noise = Mesh_GetVertexNoise(worldX, worldZ);
            height += noise;


            g_MeshVertex[index].position = { worldX, height, worldZ };
            g_MeshVertex[index].color = { 1,1,1,1 };
            g_MeshVertex[index].texcoord = { worldX * 0.1f, worldZ * 0.1f };
        } 
    }

    // NORMALS
    for (int z = 1; z < FIELD_MESH_V_VERTEX_COUNT - 1; ++z)
    {
        for (int x = 1; x < FIELD_MESH_H_VERTEX_COUNT - 1; ++x)
        {
            int i = x + z * FIELD_MESH_H_VERTEX_COUNT;
            float hl = g_MeshVertex[i - 1].position.y;
            float hr = g_MeshVertex[i + 1].position.y;
            float hd = g_MeshVertex[i - FIELD_MESH_H_VERTEX_COUNT].position.y;
            float hu = g_MeshVertex[i + FIELD_MESH_H_VERTEX_COUNT].position.y;

            XMFLOAT3 n = { hl - hr, 2.0f, hd - hu };
            XMStoreFloat3(&g_MeshVertex[i].normal,
                XMVector3Normalize(XMLoadFloat3(&n)));
        }
    }

    int k = 0;
    for (int z = 0; z < FIELD_MESH_V_COUNT; ++z)
    {
        for (int x = 0; x < FIELD_MESH_H_COUNT; ++x)
        {
            int tl = x + z * FIELD_MESH_H_VERTEX_COUNT;
            int tr = tl + 1;
            int bl = tl + FIELD_MESH_H_VERTEX_COUNT;
            int br = bl + 1;

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
    Texture_SetTexture(g_Tex0Id, 0);
    Texture_SetTexture(g_Tex1Id, 1);
 
    // Bind shadow map to t2
    ID3D11ShaderResourceView* shadowSRV = Shadow_GetShadowMap();
    g_pContext->PSSetShaderResources(2, 1, &shadowSRV);

    // Bind shadow sampler to s1
    ID3D11SamplerState* shadowSampler = Shadow_GetSampler();
    g_pContext->PSSetSamplers(1, 1, &shadowSampler);



    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;

    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ShaderField_SetWorldMatrix(XMMatrixTranslation(offsetX, heightOffset, offsetZ));
    g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
    ShaderField_SetWorldMatrix(XMMatrixIdentity());

}








