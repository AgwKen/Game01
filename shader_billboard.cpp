/*==============================================================================

  Shader BillBoard CPP [shader_billbooard.cpp]
														 Author : PYAE SONE THANT
														 Date   : 2025/11/14
--------------------------------------------------------------------------------

==============================================================================*/
#include "shader_billboard.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "debug_ostream.h"
#include <fstream>
#include "shader.h"
#include "sampler.h"

static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer0 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer3 = nullptr;//buffer3
static ID3D11Buffer* g_pPSConstantBuffer0 = nullptr;
static ID3D11PixelShader* g_pPixelShader = nullptr;



bool ShaderBillBoard_Initialize()
{
    HRESULT hr;

    // 頂点シェーダーの読み込み
    std::ifstream ifs_vs("shader_vertex_billboard.cso", std::ios::binary);
    if (!ifs_vs) {
        MessageBox(nullptr, "Shader vertex failed\n\nshader_vertex_billboard.cso", "Error", MB_OK);
        return false;
    }

    ifs_vs.seekg(0, std::ios::end);
    std::streamsize filesize = ifs_vs.tellg();
    ifs_vs.seekg(0, std::ios::beg);

    unsigned char* vsbinary_pointer = new unsigned char[filesize];
    ifs_vs.read((char*)vsbinary_pointer, filesize);
    ifs_vs.close();

    hr = Direct3D_GetDevice()->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader);

    if (FAILED(hr)) {
        hal::dout << "ShaderBillBoard_Initialize() シェーダーの読み込みに失敗しました" << std::endl;
        delete[] vsbinary_pointer;
        return false;
    }

    // 頂点レイアウトの定義
    D3D11_INPUT_ELEMENT_DESC layout[] = {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT num_elements = ARRAYSIZE(layout);
    hr = Direct3D_GetDevice()->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout);
    delete[] vsbinary_pointer;

    if (FAILED(hr)) {
        hal::dout << "ShaderBillBoard_Initialize() シェーダーの読み込みに失敗しました" << std::endl;
        return false;
    }

    // 頂点シェーダー用定数バッファ作成
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(XMFLOAT4X4);
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer0);

    buffer_desc.ByteWidth = sizeof(XMFLOAT4);
    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer3);

    // ピクセルシェーダー読み込み
    std::ifstream ifs_ps("shader_pixel_billboard.cso", std::ios::binary);
    if (!ifs_ps) {
        MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\nshader_pixel_billboard.cso", "Error", MB_OK);
        return false;
    }

    ifs_ps.seekg(0, std::ios::end);
    filesize = ifs_ps.tellg();
    ifs_ps.seekg(0, std::ios::beg);

    unsigned char* psbinary_pointer = new unsigned char[filesize];
    ifs_ps.read((char*)psbinary_pointer, filesize);
    ifs_ps.close();

    hr = Direct3D_GetDevice()->CreatePixelShader(psbinary_pointer, filesize, nullptr, &g_pPixelShader);
    delete[] psbinary_pointer;

    if (FAILED(hr)) {
        hal::dout << "ShaderBillBoard_Initialize() シェーダーの読み込みに失敗しました" << std::endl;
        return false;
    }

    // ピクセルシェーダー用定数バッファ作成
    buffer_desc.ByteWidth = sizeof(UVParameter);
    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer0);

    return true;
}

void ShaderBillBoard_Finalize()
{
    SAFE_RELEASE(g_pPixelShader);
    SAFE_RELEASE(g_pPSConstantBuffer0);
    SAFE_RELEASE(g_pVSConstantBuffer3);
    SAFE_RELEASE(g_pVSConstantBuffer0);
    SAFE_RELEASE(g_pInputLayout);
    SAFE_RELEASE(g_pVertexShader);
}

void ShaderBillBoard_SetWorldMatrix(const DirectX::XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBuffer0, 0, nullptr, &transpose, 0, 0);
}

void ShaderBillBoard_SetColor(const DirectX::XMFLOAT4& color)
{
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
}

void ShaderBillBoard_SetUVParameter(const UVParameter& parameter)
{
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBuffer3, 0, nullptr, &parameter, 0, 0);
}

void ShaderBillBoard_Begin()
{
    Direct3D_GetDeviceContext()->VSSetShader(g_pVertexShader, nullptr, 0);
    Direct3D_GetDeviceContext()->PSSetShader(g_pPixelShader, nullptr, 0);

    Direct3D_GetDeviceContext()->IASetInputLayout(g_pInputLayout);

    Direct3D_GetDeviceContext()->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer0);
    Direct3D_GetDeviceContext()->VSSetConstantBuffers(3, 1, &g_pVSConstantBuffer3);
    Direct3D_GetDeviceContext()->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);
}
