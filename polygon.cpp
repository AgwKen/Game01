/*==============================================================================

   ポリゴン描画 [polygon.cpp]

                                                         Author : Youhei Sato
                                                         Date   : 2025/05/15

--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "shader.h"
#include "debug_ostream.h"
#include "polygon.h"

#pragma comment(lib, "d3d11.lib")

using namespace DirectX;

//------------------------------------------------------------------------------
// 定数
//------------------------------------------------------------------------------
// NOTE: kept NUM_VERTEX for backward-compatibility but we compute and use g_NumVertex dynamically.
static constexpr int NUM_VERTEX = 4; // 頂点数 (not used for allocation anymore)

//------------------------------------------------------------------------------
// グローバル変数
//------------------------------------------------------------------------------
static ID3D11Buffer* g_pVertexBuffer = nullptr;             // 頂点バッファ
static ID3D11ShaderResourceView* g_pTexture = nullptr;      // テクスチャ
static ID3D11Device* g_pDevice = nullptr;                   // 初期化で外部から設定
static ID3D11DeviceContext* g_pContext = nullptr;           // 初期化で外部から設定
static int g_NumVertex = 0;         // 実際に描画する頂点数 (閉じるために +1 する場合あり)
static float g_Radius = 100.0f;
static float g_Cx = 1000.0f;
static float g_Cy = 500.0f;


//------------------------------------------------------------------------------
// 頂点構造体
//------------------------------------------------------------------------------
struct Vertex
{
    XMFLOAT3 position; // 頂点座標
    XMFLOAT4 color;    // 色
    XMFLOAT2 uv;       // UV(texcoord テクスチャー座標)
};

//------------------------------------------------------------------------------
// 初期化
//------------------------------------------------------------------------------
void Polygon_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    // デバイスとデバイスコンテキストのチェック
    if (!pDevice || !pContext) {
        hal::dout << "Polygon_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
        return;
    }

    // デバイスとデバイスコンテキストの保存
    g_pDevice = pDevice;
    g_pContext = pContext;

    // g_Radius に基づきセグメント数を決定（少なくとも 3）
    // 円周長のピクセル単位近似を使ってセグメント数を決めています。
    int segments = static_cast<int>(g_Radius * 2.0f * XM_PI);
    if (segments < 3) segments = 3;

    // We'll draw a closed line strip: allocate one extra vertex to duplicate the first vertex at the end.
    g_NumVertex = segments + 1;

    // 頂点バッファ生成（動的に書き換える想定）
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(Vertex) * g_NumVertex;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    HRESULT hr = g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
    if (FAILED(hr) || !g_pVertexBuffer) {
        hal::dout << "Polygon_Initialize() : 頂点バッファ作成失敗 hr=" << std::hex << hr << std::dec << std::endl;
        return;
    }

    // テクスチャ読み込み（既存）
    TexMetadata metadata;
    ScratchImage image;
    HRESULT loadHr = LoadFromWICFile(L"cat.png", WIC_FLAGS_NONE, &metadata, image);
    if (SUCCEEDED(loadHr)) {
        HRESULT createHr = CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_pTexture);
        if (FAILED(createHr)) {
            MessageBox(nullptr, "テクスチャの読み込みに失敗した (CreateShaderResourceView)", "エラー", MB_OK | MB_ICONERROR);
        }
    }
    else {
        MessageBox(nullptr, "テクスチャの読み込みに失敗した (LoadFromWICFile)", "エラー", MB_OK | MB_ICONERROR);
    }
}

//------------------------------------------------------------------------------
// 終了処理
//------------------------------------------------------------------------------
void Polygon_Finalize(void)
{
    SAFE_RELEASE(g_pTexture);
    SAFE_RELEASE(g_pVertexBuffer);
}

//------------------------------------------------------------------------------
// 描画
//------------------------------------------------------------------------------
void Polygon_Draw(float x, float y, float angle, float radius)
{
    if (!g_pDevice || !g_pContext || !g_pVertexBuffer) {
        return;
    }

    Shader_Begin();

    D3D11_MAPPED_SUBRESOURCE msr;
    HRESULT hr = g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    if (FAILED(hr)) {
        hal::dout << "Polygon_Draw() : VertexBuffer Map failed hr=" << std::hex << hr << std::dec << std::endl;
        return;
    }
    Vertex* v = (Vertex*)msr.pData;

    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    const int segments = 360;
    g_NumVertex = segments * 2;

    // Use the passed-in parameters for the cone's position and size.
    const XMFLOAT3 apex_position = { x, y, 0.0f };
    const XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    const float base_z = 100.0f;

    const float rad = XM_2PI / static_cast<float>(segments);
    for (int i = 0; i < segments; i++) {
        float current_angle = rad * static_cast<float>(i) + angle; // Use `angle` for rotation

        v[i * 2].position = apex_position;
        v[i * 2].color = color;
        v[i * 2].uv = { 0.5f, 0.5f };

        v[i * 2 + 1].position.x = cosf(current_angle) * radius + x;
        v[i * 2 + 1].position.y = sinf(current_angle) * radius + y;
        v[i * 2 + 1].position.z = base_z;
        v[i * 2 + 1].color = color;
        v[i * 2 + 1].uv = { (cosf(current_angle) + 1.0f) * 0.5f, (sinf(current_angle) + 1.0f) * 0.5f };
    }

    g_pContext->Unmap(g_pVertexBuffer, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    Shader_SetProjectionMatrix(XMMatrixPerspectiveFovLH(XM_PIDIV4, SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 1000.0f));

    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    g_pContext->Draw(g_NumVertex, 0);
}