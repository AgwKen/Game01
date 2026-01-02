/*==============================================================================
  Shadow Shader [shadow.cpp]
                                                         Author : PYAE SONE THANT
--------------------------------------------------------------------------------
==============================================================================*/
#include "shadow.h"
#include "direct3d.h"
#include "sampler.h"
#include <fstream>
#include <vector>

// --- D3D objects ---
static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11PixelShader* g_pPixelShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;

static ID3D11Buffer* g_pVSConstantBuffer0 = nullptr; // MatrixBuffer (b0)
static ID3D11Buffer* g_pVSConstantBuffer1 = nullptr; // LightPositionBuffer (b1)
static ID3D11Buffer* g_pPSConstantBuffer0 = nullptr; // LightBuffer (b0)

// --- Helper for creating buffers ---
static HRESULT CreateConstantBuffer(UINT size, ID3D11Buffer** ppBuffer)
{
    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = (size + 15) & ~15; // Ensure 16-byte alignment
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    return Direct3D_GetDevice()->CreateBuffer(&desc, nullptr, ppBuffer);
}

bool Shadow_Initialize()
{
    HRESULT hr;
    ID3D11Device* device = Direct3D_GetDevice();

    // --- Vertex Shader ---
    std::ifstream ifs_vs("shadow_vertex.cso", std::ios::binary);
    if (!ifs_vs) return false;
    std::vector<char> vs_data((std::istreambuf_iterator<char>(ifs_vs)), std::istreambuf_iterator<char>());
    hr = device->CreateVertexShader(vs_data.data(), vs_data.size(), nullptr, &g_pVertexShader);
    if (FAILED(hr)) return false;

    // --- Input Layout ---
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    hr = device->CreateInputLayout(layout, 3, vs_data.data(), vs_data.size(), &g_pInputLayout);
    if (FAILED(hr)) return false;

    // --- Pixel Shader ---
    std::ifstream ifs_ps("shadow_pixel.hlsl", std::ios::binary); // Ensure this points to the .cso in production
    if (!ifs_ps) return false;
    std::vector<char> ps_data((std::istreambuf_iterator<char>(ifs_ps)), std::istreambuf_iterator<char>());
    hr = device->CreatePixelShader(ps_data.data(), ps_data.size(), nullptr, &g_pPixelShader);
    if (FAILED(hr)) return false;

    // --- Constant Buffers ---
    CreateConstantBuffer(sizeof(ShadowMatrixBuffer), &g_pVSConstantBuffer0);
    CreateConstantBuffer(sizeof(ShadowLightPositionBuffer), &g_pVSConstantBuffer1);
    CreateConstantBuffer(sizeof(ShadowLightBuffer), &g_pPSConstantBuffer0);

    return true;
}

void Shadow_Finalize()
{
    if (g_pVSConstantBuffer0) g_pVSConstantBuffer0->Release();
    if (g_pVSConstantBuffer1) g_pVSConstantBuffer1->Release();
    if (g_pPSConstantBuffer0) g_pPSConstantBuffer0->Release();
    if (g_pInputLayout) g_pInputLayout->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
}

void Shadow_SetMatrices(const ShadowMatrixBuffer& matrices)
{
    ShadowMatrixBuffer temp;
    temp.world = XMMatrixTranspose(matrices.world);
    temp.view = XMMatrixTranspose(matrices.view);
    temp.projection = XMMatrixTranspose(matrices.projection);
    temp.lightView = XMMatrixTranspose(matrices.lightView);
    temp.lightProjection = XMMatrixTranspose(matrices.lightProjection);

    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBuffer0, 0, nullptr, &temp, 0, 0);
}

void Shadow_SetLightParams(const ShadowLightBuffer& lightParams)
{
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &lightParams, 0, 0);
}

void Shadow_SetLightPosition(const ShadowLightPositionBuffer& lightPos)
{
    Direct3D_GetDeviceContext()->UpdateSubresource(g_pVSConstantBuffer1, 0, nullptr, &lightPos, 0, 0);
}

void Shadow_Begin(ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* shadowMap)
{
    ID3D11DeviceContext* context = Direct3D_GetDeviceContext();

    context->IASetInputLayout(g_pInputLayout);
    context->VSSetShader(g_pVertexShader, nullptr, 0);
    context->PSSetShader(g_pPixelShader, nullptr, 0);

    // Bind Constant Buffers
    context->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer0);
    context->VSSetConstantBuffers(1, 1, &g_pVSConstantBuffer1);
    context->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);

    // Bind Textures (t0: shaderTexture, t1: depthMapTexture) 
    ID3D11ShaderResourceView* resources[] = { texture, shadowMap };
    context->PSSetShaderResources(0, 2, resources);

    // FIX: Bind Sampler States (s0: sampClamp, s1: sampWrap) 
    ID3D11SamplerState* samplers[] = { Sampler_GetClamp(), Sampler_GetWrap() };
    context->PSSetSamplers(0, 2, samplers);
}