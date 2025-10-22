/*========================================================================================


    Sampler Header [sampler.h]										        PYAE SONE THANT
                                                                           DATE:09/18/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "sampler.h"
#include "direct3d.h"


static ID3D11SamplerState* g_pSamplerFilterPoint = nullptr;
static ID3D11SamplerState* g_pSamplerFilterLinear = nullptr;
static ID3D11SamplerState* g_pSamplerFilterAnisotropic = nullptr;

static ID3D11Device* g_pDevice = nullptr;                   // �������ŊO������ݒ�
static ID3D11DeviceContext* g_pContext = nullptr;           // �������ŊO������ݒ�

void Sampler_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;

    D3D11_SAMPLER_DESC sampler_desc{};

    // �t�B���^�����O
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

    // UV�Q�ƊO�̎�舵��(UV�A�h���b�V���O���[�h)
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.BorderColor[0] = 0.0f;
    sampler_desc.BorderColor[1] = 0.0f;
    sampler_desc.BorderColor[2] = 0.0f;
    sampler_desc.BorderColor[3] = 0.0f;

    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MipLODBias = 0;
    sampler_desc.MaxAnisotropy = 16;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    g_pDevice->CreateSamplerState(&sampler_desc, &g_pSamplerFilterPoint);

    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    g_pDevice->CreateSamplerState(&sampler_desc, &g_pSamplerFilterLinear);

    sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
    g_pDevice->CreateSamplerState(&sampler_desc, &g_pSamplerFilterAnisotropic);
}

void Sampler_Finalize()
{
    SAFE_RELEASE(g_pSamplerFilterAnisotropic);
    SAFE_RELEASE(g_pSamplerFilterLinear);
    SAFE_RELEASE(g_pSamplerFilterPoint);
}

void Sampler_SetFilterPoint()
{
    g_pContext->PSSetSamplers(0, 1, &g_pSamplerFilterPoint);
}

void Sampler_SetFilterLinear()
{
    g_pContext->PSSetSamplers(0, 1, &g_pSamplerFilterLinear);
}

void Sampler_SetFilterAnisotropic()
{
    g_pContext->PSSetSamplers(0, 1, &g_pSamplerFilterAnisotropic);
}
