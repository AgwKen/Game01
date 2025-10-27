    /*==============================================================================

  3D shader  [shader3d.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/09/10
--------------------------------------------------------------------------------

==============================================================================*/
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "debug_ostream.h"
#include <fstream>
#include <Windows.h>
#include "shader.h"
#include "shader3d.h"
#include "sampler.h"


static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer0 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer1 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer2 = nullptr;
static ID3D11Buffer* g_pPSConstantBuffer0 = nullptr;
static ID3D11PixelShader* g_pPixelShader = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


bool Shader3d_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    HRESULT hr;

    if (!pDevice || !pContext) {
        hal::dout << "Shader3d_Initialize() have failed" << std::endl;
        return false;
    }

    g_pDevice = pDevice;
    g_pContext = pContext;

    // 頂点シェーダーの読み込み
    std::ifstream ifs_vs("shader_vertex_3d.cso", std::ios::binary);
    if (!ifs_vs) {
        MessageBox(nullptr, "Shader vertex failed\n\nshader_vertex_3d.cso", "Error", MB_OK);
        return false;
    }

    ifs_vs.seekg(0, std::ios::end);
    std::streamsize filesize = ifs_vs.tellg();
    ifs_vs.seekg(0, std::ios::beg);

    unsigned char* vsbinary_pointer = new unsigned char[filesize];
    ifs_vs.read((char*)vsbinary_pointer, filesize);
    ifs_vs.close();

    hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader);
    if (FAILED(hr)) {
        hal::dout << "Shader_Initialize() シェーダーの読み込みに失敗しました" << std::endl;
        delete[] vsbinary_pointer;
        return false;
    }

    // 頂点レイアウトの定義
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT num_elements = ARRAYSIZE(layout);
    hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout);
    delete[] vsbinary_pointer;

    if (FAILED(hr)) {
        hal::dout << "Shader_Initialize() シェーダーの読み込みに失敗しました" << std::endl;
        return false;
    }

    // 頂点シェーダー用定数バッファ作成
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(XMFLOAT4X4);
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer0);
    g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer1);
    g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer2);

    // ピクセルシェーダー読み込み
    std::ifstream ifs_ps("shader_pixel_3d.cso", std::ios::binary);
    if (!ifs_ps) {
        MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\nshader_pixel_3d.cso", "Error", MB_OK);
        return false;
    }

    ifs_ps.seekg(0, std::ios::end);
    filesize = ifs_ps.tellg();
    ifs_ps.seekg(0, std::ios::beg);

    unsigned char* psbinary_pointer = new unsigned char[filesize];
    ifs_ps.read((char*)psbinary_pointer, filesize);
    ifs_ps.close();

    hr = g_pDevice->CreatePixelShader(psbinary_pointer, filesize, nullptr, &g_pPixelShader);
    delete[] psbinary_pointer;

    if (FAILED(hr)) {
        hal::dout << "Shader_Initialize() シェーダーの読み込みに失敗しました" << std::endl;
        return false;
    }

    // ピクセルシェーダー用定数バッファ作成
    buffer_desc.ByteWidth = sizeof(XMFLOAT4);
    g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer0);

    return true;
}

void Shader3d_Finalize()
{
    SAFE_RELEASE(g_pPixelShader);
    SAFE_RELEASE(g_pPSConstantBuffer0);
    SAFE_RELEASE(g_pVSConstantBuffer2);
    SAFE_RELEASE(g_pVSConstantBuffer1);
    SAFE_RELEASE(g_pVSConstantBuffer0);
    SAFE_RELEASE(g_pInputLayout);
    SAFE_RELEASE(g_pVertexShader);
}

void Shader3d_SetWorldMatrix(const DirectX::XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    g_pContext->UpdateSubresource(g_pVSConstantBuffer0, 0, nullptr, &transpose, 0, 0);
}

void Shader3d_SetViewMatrix(const DirectX::XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    g_pContext->UpdateSubresource(g_pVSConstantBuffer1, 0, nullptr, &transpose, 0, 0);
}

void Shader3d_SetProjectionMatrix(const DirectX::XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    g_pContext->UpdateSubresource(g_pVSConstantBuffer2, 0, nullptr, &transpose, 0, 0);
}

void Shader3d_SetColor(const XMFLOAT4& color)
{
    g_pContext->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
}

void Shader3d_Begin()
{
    g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pContext->IASetInputLayout(g_pInputLayout);

    g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer0);
    g_pContext->VSSetConstantBuffers(1, 1, &g_pVSConstantBuffer1);
    g_pContext->VSSetConstantBuffers(2, 1, &g_pVSConstantBuffer2);

    g_pContext->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);

    Sampler_SetFilterAnisotropic();
}
