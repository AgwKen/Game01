/*========================================================================================

    Mesh CPP [mesh.cpp]                                         PYAE SONE THANT
                                                                DATE:09/24/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "Mesh.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader_field.h"
#include "texture.h"
#include "camera.h"

static constexpr float FIELD_MESH_SIZE = 1.0f;
static constexpr int FIELD_MESH_H_COUNT = 50;
static constexpr int FIELD_MESH_V_COUNT = 25;
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

struct Vertex3d
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT4 color;
    XMFLOAT2 texcoord;
};

static Vertex3d g_MeshVertex[NUM_VERTEX];
static unsigned short g_MeshIndex[NUM_INDEX];

void Mesh_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;


    for (int z = 0; z < FIELD_MESH_V_VERTEX_COUNT; z++) {
        for (int x = 0; x < FIELD_MESH_H_VERTEX_COUNT; x++) {
            int index = x + z * FIELD_MESH_H_VERTEX_COUNT;
            g_MeshVertex[index].position = { x * FIELD_MESH_SIZE, 0.0f, z * FIELD_MESH_SIZE };
			g_MeshVertex[index].normal = { 0.0f, 1.0f, 0.0f };
            g_MeshVertex[index].color = { 0.0f, 1.0f, 1.0f, 1.0f };
            g_MeshVertex[index].texcoord = { x * 1.0f,z * 1.0f };
        }
    }


    int k = 0;
    for (int z = 0; z < FIELD_MESH_V_COUNT; z++) {
        for (int x = 0; x < FIELD_MESH_H_COUNT; x++) {
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


    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = g_MeshVertex;
    g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

    bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    sd.pSysMem = g_MeshIndex;
    g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);

    g_Tex0Id = Texture_Load(L"Texture/grass.jpg");
    g_Tex1Id = Texture_Load(L"Texture/stone.jpg");

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

    ShaderField_SetViewMatrix(XMLoadFloat4x4(&Camera_GetMatrix()));
    ShaderField_SetProjectionMatrix(XMLoadFloat4x4(&Camera_GetPerspectiveMatrix()));

    for (int z = 0; z < repeatZ; z++) {
        for (int x = 0; x < repeatX; x++) {
            float worldX = x * FIELD_MESH_H_COUNT * FIELD_MESH_SIZE + offsetX;
            float worldZ = z * FIELD_MESH_V_COUNT * FIELD_MESH_SIZE + offsetZ;

            ShaderField_SetWorldMatrix(XMMatrixTranslation(worldX, heightOffset, worldZ));

            g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
        }
    }
}