/*==============================================================================

  No Light 3D Shader [shader3d_unlit.cpp]
  FIXED VERSION (World + View + Projection)

==============================================================================*/

#include "shader3d_unlit.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "debug_ostream.h"
#include <fstream>
#include "shader.h"
#include "sampler.h"

static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;

static ID3D11Buffer* g_pVSConstantBufferWorld = nullptr;
static ID3D11Buffer* g_pVSConstantBufferView = nullptr;
static ID3D11Buffer* g_pVSConstantBufferProj = nullptr;

static ID3D11Buffer* g_pPSConstantBuffer0 = nullptr;

static ID3D11PixelShader* g_pPixelShader = nullptr;


bool Shader3dUnlit_Initialize()
{
    HRESULT hr;

    // ===============================
    // Load Vertex Shader
    // ===============================
    std::ifstream ifs_vs("shader_vertex_3d_unlit.cso", std::ios::binary);
    if (!ifs_vs)
    {
        MessageBox(nullptr, "shader_vertex_3d_unlit.cso missing", "Error", MB_OK);
        return false;
    }

    ifs_vs.seekg(0, std::ios::end);
    std::streamsize filesize = ifs_vs.tellg();
    ifs_vs.seekg(0, std::ios::beg);

    unsigned char* vsbinary = new unsigned char[filesize];
    ifs_vs.read((char*)vsbinary, filesize);
    ifs_vs.close();

    hr = Direct3D_GetDevice()->CreateVertexShader(vsbinary, filesize, nullptr, &g_pVertexShader);
    if (FAILED(hr))
    {
        delete[] vsbinary;
        return false;
    }

    // ===============================
    // Input Layout
    // ===============================
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = Direct3D_GetDevice()->CreateInputLayout(
        layout,
        _countof(layout),
        vsbinary,
        filesize,
        &g_pInputLayout
    );

    delete[] vsbinary;

    if (FAILED(hr))
        return false;

    // ===============================
    // Create VS Constant Buffers
    // ===============================
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = sizeof(XMFLOAT4X4);
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBufferWorld);
    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBufferView);
    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBufferProj);

    // ===============================
    // Load Pixel Shader
    // ===============================
    std::ifstream ifs_ps("shader_pixel_3d_unlit.cso", std::ios::binary);
    if (!ifs_ps)
    {
        MessageBox(nullptr, "shader_pixel_3d_unlit.cso missing", "Error", MB_OK);
        return false;
    }

    ifs_ps.seekg(0, std::ios::end);
    filesize = ifs_ps.tellg();
    ifs_ps.seekg(0, std::ios::beg);

    unsigned char* psbinary = new unsigned char[filesize];
    ifs_ps.read((char*)psbinary, filesize);
    ifs_ps.close();

    hr = Direct3D_GetDevice()->CreatePixelShader(psbinary, filesize, nullptr, &g_pPixelShader);
    delete[] psbinary;

    if (FAILED(hr))
        return false;

    // ===============================
    // Create PS Constant Buffer
    // ===============================
    buffer_desc.ByteWidth = sizeof(XMFLOAT4);
    Direct3D_GetDevice()->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer0);

    return true;
}


void Shader3dUnlit_Finalize()
{
    SAFE_RELEASE(g_pPixelShader);
    SAFE_RELEASE(g_pPSConstantBuffer0);

    SAFE_RELEASE(g_pVSConstantBufferProj);
    SAFE_RELEASE(g_pVSConstantBufferView);
    SAFE_RELEASE(g_pVSConstantBufferWorld);

    SAFE_RELEASE(g_pInputLayout);
    SAFE_RELEASE(g_pVertexShader);
}


// ===============================
// Matrix Setters
// ===============================

void Shader3dUnlit_SetWorldMatrix(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBufferWorld, 0, nullptr, &transpose, 0, 0);
}

void Shader3dUnlit_SetViewMatrix(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBufferView, 0, nullptr, &transpose, 0, 0);
}

void Shader3dUnlit_SetProjectionMatrix(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBufferProj, 0, nullptr, &transpose, 0, 0);
}


// ===============================
// Color Setter
// ===============================

void Shader3dUnlit_SetColor(const XMFLOAT4& color)
{
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
}


// ===============================
// Begin
// ===============================

void Shader3dUnlit_Begin()
{
    ID3D11DeviceContext* context = Direct3D_GetDeviceContext();

    context->VSSetShader(g_pVertexShader, nullptr, 0);
    context->PSSetShader(g_pPixelShader, nullptr, 0);

    context->IASetInputLayout(g_pInputLayout);

    ID3D11Buffer* vsBuffers[3] =
    {
        g_pVSConstantBufferWorld,
        g_pVSConstantBufferView,
        g_pVSConstantBufferProj
    };

    context->VSSetConstantBuffers(0, 3, vsBuffers);
    context->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);
}