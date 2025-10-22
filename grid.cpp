/*========================================================================================

    Grid CPP [grid.cpp]                                        PYAE SONE THANT
                                                               DATE:09/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "grid.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader3d.h"
#include "texture.h"

static constexpr int GRID_H_COUNT = 10;
static constexpr int GRID_V_COUNT = 10;
static constexpr int GRID_H_LINE_COUNT = GRID_H_COUNT + 1;
static constexpr int GRID_V_LINE_COUNT = GRID_V_COUNT + 1;
static constexpr int NUM_VERTEX = GRID_H_LINE_COUNT * 2 + GRID_V_LINE_COUNT * 2;

static ID3D11Buffer* g_pVertexBuffer = nullptr;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// Add a static pointer for our solid color texture view
static ID3D11ShaderResourceView* g_pSolidColorSRV = nullptr;

// 3D頂点構造体
struct Vertex3d
{
    XMFLOAT3 position; // 頂点座標
    XMFLOAT4 color;    // 色
    XMFLOAT2 uv;       // Add UV coordinates
};

static Vertex3d g_GridVertex[NUM_VERTEX] = {};


void Grid_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;

    // Create a 1x1 green texture for the grid
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = 1;
    textureDesc.Height = 1;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    XMFLOAT4 solidGreenColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &solidGreenColor;
    initData.SysMemPitch = sizeof(XMFLOAT4);

    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr = g_pDevice->CreateTexture2D(&textureDesc, &initData, &pTexture);
    if (SUCCEEDED(hr))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

        g_pDevice->CreateShaderResourceView(pTexture, &srvDesc, &g_pSolidColorSRV);
    }
    SAFE_RELEASE(pTexture);

    // 頂点データ作成
    int index = 0;
    // Set the color to white so it doesn't tint the green texture
    XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

    constexpr float GRID_WIDTH = 10.0f;
    constexpr float GRID_DEPTH = 10.0f;

    float dx = GRID_WIDTH / GRID_H_COUNT;
    float dz = GRID_DEPTH / GRID_V_COUNT;

    float startX = -GRID_WIDTH * 0.5f;
    float endX = GRID_WIDTH * 0.5f;
    float startZ = -GRID_DEPTH * 0.5f;
    float endZ = GRID_DEPTH * 0.5f;

    // UV coordinates for a 1x1 texture should be constant
    XMFLOAT2 uv = { 0.5f, 0.5f };

    // Vertical lines (along Z axis)
    for (int i = 0; i <= GRID_H_COUNT; i++)
    {
        float x = startX + i * dx;
        g_GridVertex[index++] = { { x, 0.0f, startZ }, color, uv };
        g_GridVertex[index++] = { { x, 0.0f, endZ }, color, uv };
    }

    // Horizontal lines (along X axis)
    for (int j = 0; j <= GRID_V_COUNT; j++)
    {
        float z = startZ + j * dz;
        g_GridVertex[index++] = { { startX, 0.0f, z }, color, uv };
        g_GridVertex[index++] = { { endX, 0.0f, z }, color, uv };
    }

    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = g_GridVertex;

    g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);
}

void Grid_Finalize(void)
{
    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pSolidColorSRV); // Release the new resource
}

void Grid_Draw(void)
{
    Shader3d_Begin();

    // Bind the 1x1 green texture. The pixel shader will sample from this.
    // The second parameter is the shader slot (usually 0)
    g_pContext->PSSetShaderResources(0, 1, &g_pSolidColorSRV);

    // Set the vertex buffer to the drawing pipeline
    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    // Create a world coordinate transformation matrix
    XMMATRIX mtxWorld = XMMatrixIdentity();

    // Set the projection transformation matrix in the vertex shader
    Shader3d_SetWorldMatrix(mtxWorld);

    // Primitive topology settings
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Draw all grid vertices
    g_pContext->Draw(NUM_VERTEX, 0);
}