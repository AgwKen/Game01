/*========================================================================================


    Mesh cpp [Mesh.cpp]									    	        PYAE SONE THANT
                                                                        DATE:09/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "Mesh.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader_field.h"
#include "texture.h"
#include "camera.h"
#include <cmath>

static constexpr float FIELD_MESH_SIZE = 1.0f;
static constexpr int FIELD_MESH_H_COUNT = 100;
static constexpr int FIELD_MESH_V_COUNT = 100;
static constexpr int FIELD_MESH_H_VERTEX_COUNT = FIELD_MESH_H_COUNT + 1;
static constexpr int FIELD_MESH_V_VERTEX_COUNT = FIELD_MESH_V_COUNT + 1;
static constexpr int NUM_VERTEX = FIELD_MESH_H_VERTEX_COUNT * FIELD_MESH_V_VERTEX_COUNT;
static constexpr int NUM_INDEX = 3 * 2 * FIELD_MESH_H_COUNT * FIELD_MESH_V_COUNT;

static ID3D11Buffer* g_pVertexBuffer = nullptr;
static ID3D11Buffer* g_pIndexBuffer = nullptr;

static int g_Tex0Id = -1;
static int g_Tex1Id = -1;

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

struct Cone
{
    float x, z;    // center
    float radius;
    float height;
};

static Cone g_Mountains[] = {
{20.0f, 15.0f, 8.0f, 3.5f},
{50.0f, 30.0f, 12.0f, 6.0f},
{70.0f, 10.0f, 10.0f, 5.0f},
};
static int g_NumMountains = sizeof(g_Mountains) / sizeof(Cone);

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

            // --- Cone mountains ---
            for (int m = 0; m < g_NumMountains; ++m)
            {
                float dx = worldX - g_Mountains[m].x;
                float dz = worldZ - g_Mountains[m].z;
                float dist = sqrtf(dx * dx + dz * dz);
                if (dist < g_Mountains[m].radius)
                    height += g_Mountains[m].height * (1.0f - dist / g_Mountains[m].radius);
            }

            /** -- - Ridge-- -
            {
                float ridgeX = 0.0f;
                float ridgeZ = 0.0f;
                float ridgeLength = 150.0f;
                float ridgeWidth = 10.0f;
                float ridgeHeight = 6.0f;

                float dx = worldX - ridgeX;
                float dz = worldZ - ridgeZ;

                float ridge = ridgeHeight * expf(-(dx * dx) / (ridgeWidth * ridgeWidth));

                float zFactor = 1.0f;
                if (dz < 0 || dz > ridgeLength) zFactor = 0.0f;
                else if (dz < 10.0f) zFactor = dz / 10.0f;
                else if (dz > ridgeLength - 10.0f) zFactor = (ridgeLength - dz) / 10.0f;

                height += ridge * zFactor;
            }
            */
            // --- Plateau ---
            {
                float plateX = 50.0f;
                float plateZ = 80.0f;
                float plateRadius = 20.0f;
                float plateHeight = 5.0f;
                float edgeFalloff = 5.0f;

                float dx = worldX - plateX;
                float dz = worldZ - plateZ;
                float dist = sqrtf(dx * dx + dz * dz);

                if (dist < plateRadius)
                    height += plateHeight;
                else if (dist < plateRadius + edgeFalloff)
                    height += plateHeight * (1.0f - (dist - plateRadius) / edgeFalloff);
            }

            // --- Valley / Depression ---
            {
                float valX = 120.0f;
                float valZ = 200.0f;
                float valRadius = 15.0f;
                float valDepth = 4.0f;

                float dx = worldX - valX;
                float dz = worldZ - valZ;
                float dist = sqrtf(dx * dx + dz * dz);

                if (dist < valRadius)
                    height -= valDepth * (1.0f - dist / valRadius);
            }

            /* -- - Crater / Hole-- -
            {
                float crX = 40.0f;
                float crZ = 50.0f;
                float crRadius = 8.0f;
                float crDepth = 3.0f;

                float dx = worldX - crX;
                float dz = worldZ - crZ;
                float dist = sqrtf(dx * dx + dz * dz);

                if (dist < crRadius)
                    height -= crDepth * (1.0f - dist / crRadius);
            }
            */
            // --- Terraces / Steps ---
            //{
            //    float stepHeight = 2.0f;
            //    height = floor(height / stepHeight) * stepHeight;
            //}

            // --- Random Noise ---
            {
                float noise = ((rand() % 20) / 100.0f - 0.5f) * 0.5f; // -0.25 to 0.25
                height += noise;
            }
            // --- River / River Bank ---
            {
                float riverCenterX = 100.0f;   // X position of river center
                float riverStartZ = 0.0f;      // start Z
                float riverEndZ = 300.0f;      // end Z
                float riverWidth = 5.0f;       // half-width of river
                float riverDepth = 3.0f;       // depth of river

                // Check if vertex is within river length
                if (worldZ >= riverStartZ && worldZ <= riverEndZ)
                {
                    float dx = worldX - riverCenterX;
                    if (fabs(dx) < riverWidth)
                    {
                        // Linear slope to river center
                        float factor = 1.0f - fabs(dx) / riverWidth; // 0 at edge, 1 at center
                        height -= riverDepth * factor;  // lower height
                    }
                }
            }


            // Set vertex
            g_MeshVertex[index].position = { worldX, height, worldZ };
            g_MeshVertex[index].normal = { 0.0f, 1.0f, 0.0f };
            g_MeshVertex[index].color = { 0.0f, 1.0f, 1.0f, 1.0f };
            g_MeshVertex[index].texcoord = { float(x), float(z) };
        }
    }
    // --- Index buffer (unchanged) ---
    int k = 0;
    for (int z = 0; z < FIELD_MESH_V_COUNT; ++z)
    {
        for (int x = 0; x < FIELD_MESH_H_COUNT; ++x)
        {
            int topLeft = x + z * FIELD_MESH_H_VERTEX_COUNT;
            int topRight = (x + 1) + z * FIELD_MESH_H_VERTEX_COUNT;
            int bottomLeft = x + (z + 1) * FIELD_MESH_H_VERTEX_COUNT;
            int bottomRight = (x + 1) + (z + 1) * FIELD_MESH_H_VERTEX_COUNT;

            g_MeshIndex[k++] = topLeft;
            g_MeshIndex[k++] = bottomLeft;
            g_MeshIndex[k++] = topRight;

            g_MeshIndex[k++] = bottomLeft;
            g_MeshIndex[k++] = bottomRight;
            g_MeshIndex[k++] = topRight;
        }
    }

    // --- Create buffers ---
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

    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw single map only
    ShaderField_SetWorldMatrix(XMMatrixTranslation(offsetX, heightOffset, offsetZ));
    g_pContext->DrawIndexed(NUM_INDEX, 0, 0);

}

float Mesh_GetHeightAt(float worldX, float worldZ)
{
    const float TILE_SIZE_X = FIELD_MESH_H_COUNT * FIELD_MESH_SIZE;
    const float TILE_SIZE_Z = FIELD_MESH_V_COUNT * FIELD_MESH_SIZE;

    float localX = worldX - g_CollisionOffsetX;
    float localZ = worldZ - g_CollisionOffsetZ;

    if (localX < 0.0f || localZ < 0.0f)
        return 0.0f;

    int tx = (int)(localX / TILE_SIZE_X);
    int tz = (int)(localZ / TILE_SIZE_Z);

    if (tx < 0 || tx >= g_RepeatX || tz < 0 || tz >= g_RepeatZ)
        return 0.0f;

    float meshX = localX - tx * TILE_SIZE_X;
    float meshZ = localZ - tz * TILE_SIZE_Z;

    if (meshX < 0.0f) meshX = 0.0f;
    if (meshZ < 0.0f) meshZ = 0.0f;

    const float EPS = 0.0001f;
    if (meshX > TILE_SIZE_X - EPS) meshX = TILE_SIZE_X - EPS;
    if (meshZ > TILE_SIZE_Z - EPS) meshZ = TILE_SIZE_Z - EPS;

 
    int ix = (int)(meshX / FIELD_MESH_SIZE);
    int iz = (int)(meshZ / FIELD_MESH_SIZE);

    if (ix < 0) ix = 0;
    if (iz < 0) iz = 0;
    if (ix >= FIELD_MESH_H_COUNT) ix = FIELD_MESH_H_COUNT - 1;
    if (iz >= FIELD_MESH_V_COUNT) iz = FIELD_MESH_V_COUNT - 1;

    float fx = (meshX - ix * FIELD_MESH_SIZE) / FIELD_MESH_SIZE;
    float fz = (meshZ - iz * FIELD_MESH_SIZE) / FIELD_MESH_SIZE;

    int v0 = ix + iz * FIELD_MESH_H_VERTEX_COUNT;
    int v1 = (ix + 1) + iz * FIELD_MESH_H_VERTEX_COUNT;
    int v2 = ix + (iz + 1) * FIELD_MESH_H_VERTEX_COUNT;
    int v3 = (ix + 1) + (iz + 1) * FIELD_MESH_H_VERTEX_COUNT;

    float h0 = g_MeshVertex[v0].position.y * (1.0f - fx) + g_MeshVertex[v1].position.y * fx;
    float h1 = g_MeshVertex[v2].position.y * (1.0f - fx) + g_MeshVertex[v3].position.y * fx;
    float height = h0 * (1.0f - fz) + h1 * fz;

    // Cone mountains
    for (int m = 0; m < g_NumMountains; ++m)
    {
        float dx = worldX - g_Mountains[m].x;
        float dz = worldZ - g_Mountains[m].z;
        float dist = sqrtf(dx * dx + dz * dz);

        if (dist < g_Mountains[m].radius)
        {
            float cone = g_Mountains[m].height * (1.0f - dist / g_Mountains[m].radius);
            if (cone > height)
                height = cone;
        }
    }

    return height;
}
