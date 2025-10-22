/*========================================================================================


    Cube CPP [grid.cpp]                                                 PYAE SONE THANT
                                                                        DATE:09/12/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "cube.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader3d.h"
#include "texture.h"

static constexpr int NUM_VERTEX = 4 * 6; 
static constexpr int NUM_INDEX = 3 * 2 * 6;

static ID3D11Buffer* g_pVertexBuffer = nullptr;             // 頂点バッファ
static ID3D11Buffer* g_pIndexBuffer = nullptr;             // INDEXバッファ

static int g_CubeTexId = -1; // テクスチャ番号

static ID3D11Device* g_pDevice = nullptr;                   // 初期化で外部から設定
static ID3D11DeviceContext* g_pContext = nullptr;           // 初期化で外部から設定

// 3D頂点構造体
struct Vertex3d
{
    XMFLOAT3 position; // 頂点座標
    XMFLOAT3 normal;
    XMFLOAT4 color;    // 色
	XMFLOAT2 texcoord; // UV座標
};

static Vertex3d g_CubeVertex[] =
{
    // Front face (z = -0.5, normal = {0.0f, 0.0f, -1.0f})
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 0.0f}}, // 0 top-left
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 0.0f}}, // 1 top-right
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 1.0f}}, // 2 bottom-right
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 1.0f}}, // 3 bottom-left

    // Back face (z = +0.5, normal = {0.0f, 0.0f, 1.0f})
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 0.0f}}, // 4 top-left
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 0.0f}}, // 5 top-right
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 1.0f}}, // 6 bottom-right
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 1.0f}}, // 7 bottom-left

    // Top face (y = +0.5, normal = {0.0f, 1.0f, 0.0f})
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 0.0f}}, // 8 front-left
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 0.0f}}, // 9 front-right
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 1.0f}}, // 10 back-right
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 1.0f}}, // 11 back-left

    // Bottom face (y = -0.5, normal = {0.0f, -1.0f, 0.0f})
    {{-0.5f, -0.5f, -0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 0.0f}}, // 12 back-left
    {{ 0.5f, -0.5f, -0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 0.0f}}, // 13 back-right
    {{ 0.5f, -0.5f,  0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 1.0f}}, // 14 front-right
    {{-0.5f, -0.5f,  0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 1.0f}}, // 15 front-left

    // Left face (x = -0.5, normal = {-1.0f, 0.0f, 0.0f})
    {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 0.0f}}, // 16 top-front
    {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 0.0f}}, // 17 top-back
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 1.0f}}, // 18 bottom-back
    {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 1.0f}}, // 19 bottom-front

    // Right face (x = +0.5, normal = {1.0f, 0.0f, 0.0f})
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 0.0f}}, // 20 top-back
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 0.0f}}, // 21 top-front
    {{ 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f, 1.0f}}, // 22 bottom-front
    {{ 0.5f, -0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f, 1.0f}}, // 23 bottom-back
};

static unsigned short g_CubeIndex[] =
{
    0, 1, 2, 0, 2, 3,        // Front
    4, 5, 6, 4, 6, 7,        // Back
    8, 9,10, 8,10,11,        // Top
   12,13,14,12,14,15,        // Bottom
   16,17,18,16,18,19,        // Left
   20,21,22,20,22,23         // Right
};

void CUBE_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;

    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX; //sizeof(g_CubeVertex);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = g_CubeVertex;

    g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	//Index Buffer生成
	bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX; //sizeof(g_CubeIndex);
    bd.CPUAccessFlags = 0;

    sd.pSysMem = g_CubeIndex;

    g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);


    g_CubeTexId = Texture_Load(L"Texture/poop.png");
}

void CUBE_Finalize(void)
{
    SAFE_RELEASE(g_pIndexBuffer);
    SAFE_RELEASE(g_pVertexBuffer);
}

void CUBE_Draw(float angle)
{
    Shader3d_Begin();

	Shader3d_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

    // Set the blend state to opaque
    Direct3D_SetDefaultBlendState();

    Texture_SetTexture(g_CubeTexId);

    UINT stride = sizeof(Vertex3d), offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float y = 0.5f;
    float baseY = 0.5f;
    float scales[] = { 3.0f, 2.5f, 2.0f, 1.5f, 1.0f };

    for (int i = 0; i < 5; i++)
    {
        XMMATRIX mtxWorld =
            XMMatrixScaling(scales[i], 0.5f, scales[i]) *
            XMMatrixRotationY(angle) *
            XMMatrixTranslation(0, baseY + y * i, 0.5f);

        Shader3d_SetWorldMatrix(mtxWorld);

		g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
    }
}