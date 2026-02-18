#include "shadow.h"
#include "direct3d.h"

static ID3D11Texture2D* g_pShadowTexture = nullptr;
static ID3D11DepthStencilView* g_pShadowDSV = nullptr;
static ID3D11ShaderResourceView* g_pShadowSRV = nullptr;
static ID3D11SamplerState* g_pShadowSampler = nullptr;

bool Shadow_Initialize()
{
    ID3D11Device* device = Direct3D_GetDevice();
    if (!device) return false;

    HRESULT hr;

    // Create depth texture
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 4096;
    depthDesc.Height = 4096;

    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    hr = device->CreateTexture2D(&depthDesc, nullptr, &g_pShadowTexture);
    if (FAILED(hr)) return false;

    // Depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    hr = device->CreateDepthStencilView(g_pShadowTexture, &dsvDesc, &g_pShadowDSV);
    if (FAILED(hr)) return false;

    // Shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(
        g_pShadowTexture,
        &srvDesc,
        &g_pShadowSRV
    );
    if (FAILED(hr)) return false;


    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

sampDesc.BorderColor[0] = 1.0f;
sampDesc.BorderColor[1] = 1.0f;
sampDesc.BorderColor[2] = 1.0f;
sampDesc.BorderColor[3] = 1.0f;

    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    device->CreateSamplerState(&sampDesc, &g_pShadowSampler);



    return true;

}

void Shadow_Finalize()
{
    if (g_pShadowSampler) { g_pShadowSampler->Release(); g_pShadowSampler = nullptr; }
    if (g_pShadowSRV) { g_pShadowSRV->Release(); g_pShadowSRV = nullptr; }
    if (g_pShadowDSV) { g_pShadowDSV->Release(); g_pShadowDSV = nullptr; }
    if (g_pShadowTexture) { g_pShadowTexture->Release(); g_pShadowTexture = nullptr; }
}

ID3D11DepthStencilView* Shadow_GetDSV()
{
    return g_pShadowDSV;
}

ID3D11ShaderResourceView* Shadow_GetShadowMap()
{
    return g_pShadowSRV;
}

void Shadow_SetRenderTarget()
{
    ID3D11DeviceContext* context = Direct3D_GetDeviceContext();

    context->OMSetRenderTargets(0, nullptr, g_pShadowDSV);

    D3D11_VIEWPORT vp = {};
    vp.Width = 4096;
    vp.Height = 4096;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);
}

void Shadow_Clear()
{
    ID3D11DeviceContext* context = Direct3D_GetDeviceContext();
    context->ClearDepthStencilView(g_pShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11SamplerState* Shadow_GetSampler()
{
    return g_pShadowSampler;
}

