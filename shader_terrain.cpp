/*==============================================================================

  Shader Terrain CPP [shader_terrain.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/24/11
--------------------------------------------------------------------------------

==============================================================================*/
#include "shader_terrain.h"
#include "direct3d.h"
#include "debug_ostream.h"
#include <fstream>
using namespace DirectX;

static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11PixelShader* g_pPixelShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;

static ID3D11Buffer* g_pVSConstantBuffer_World = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer_View = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer_Proj = nullptr;

bool ShaderTerrain_Initialize()
{
    HRESULT hr;

    // --- Vertex Shader ---
    std::ifstream ifs_vs("shader_vertex_terrain.cso", std::ios::binary);
    if (!ifs_vs) {
        MessageBox(nullptr, "Shader vertex terrain load failed\n\nshader_vertex_terrain.cso", "Error", MB_OK);
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
        hal::dout << "ShaderTerrain_Initialize() Vertex Shader failed" << std::endl;
        delete[] vsbinary_pointer;
        return false;
    }

    // Input Layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = Direct3D_GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), vsbinary_pointer, filesize, &g_pInputLayout);
    delete[] vsbinary_pointer;

    if (FAILED(hr)) {
        hal::dout << "ShaderTerrain_Initialize() Input Layout failed" << std::endl;
        return false;
    }

    // --- Constant Buffers ---
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = sizeof(XMFLOAT4X4);
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_World);
    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_View);
    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_Proj);

    // --- Pixel Shader ---
    std::ifstream ifs_ps("shader_pixel_terrain.cso", std::ios::binary);
    if (!ifs_ps) {
        MessageBox(nullptr, "Pixel Shader terrain load failed\n\nshader_pixel_terrain.cso", "Error", MB_OK);
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
        hal::dout << "ShaderTerrain_Initialize() Pixel Shader failed" << std::endl;
        return false;
    }

    return true;
}

void ShaderTerrain_Finalize()
{
    SAFE_RELEASE(g_pPixelShader);
    SAFE_RELEASE(g_pVertexShader);
    SAFE_RELEASE(g_pInputLayout);
    SAFE_RELEASE(g_pVSConstantBuffer_World);
    SAFE_RELEASE(g_pVSConstantBuffer_View);
    SAFE_RELEASE(g_pVSConstantBuffer_Proj);
}

void ShaderTerrain_SetWorldMatrix(const XMMATRIX& matrix)
{
    XMFLOAT4X4 m{};
    XMStoreFloat4x4(&m, XMMatrixTranspose(matrix));
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBuffer_World, 0, nullptr, &m, 0, 0);
}

void ShaderTerrain_SetViewMatrix(const XMMATRIX& matrix)
{
    XMFLOAT4X4 m{};
    XMStoreFloat4x4(&m, XMMatrixTranspose(matrix));
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBuffer_View, 0, nullptr, &m, 0, 0);
}

void ShaderTerrain_SetProjectionMatrix(const XMMATRIX& matrix)
{
    XMFLOAT4X4 m{};
    XMStoreFloat4x4(&m, XMMatrixTranspose(matrix));
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBuffer_Proj, 0, nullptr, &m, 0, 0);
}

void ShaderTerrain_Begin()
{
    Direct3D_GetDeviceContext()->VSSetShader(g_pVertexShader, nullptr, 0);
    Direct3D_GetDeviceContext()->PSSetShader(g_pPixelShader, nullptr, 0);
    Direct3D_GetDeviceContext()->IASetInputLayout(g_pInputLayout);

    Direct3D_GetDeviceContext()->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer_World);
    Direct3D_GetDeviceContext()->VSSetConstantBuffers(1, 1, &g_pVSConstantBuffer_View);
    Direct3D_GetDeviceContext()->VSSetConstantBuffers(2, 1, &g_pVSConstantBuffer_Proj);
}
