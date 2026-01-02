/*==============================================================================
  Shadow Shader [shadow.h]
                                                         Author : PYAE SONE THANT
--------------------------------------------------------------------------------
==============================================================================*/
#ifndef SHADOW_H
#define SHADOW_H

#include <d3d11.h>
#include <DirectXMath.h>

// ---------------------------------------------------------
// Constant Buffer Structures
// ---------------------------------------------------------
struct ShadowMatrixBuffer
{
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX lightView;
    DirectX::XMMATRIX lightProjection;
};

struct ShadowLightBuffer
{
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 diffuseColor;
    float bias;
    DirectX::XMFLOAT3 lightPadding; // Match HLSL padding
};

struct ShadowLightPositionBuffer
{
    DirectX::XMFLOAT3 lightPosition;
    float padding;
};

// ---------------------------------------------------------
// Functions
// ---------------------------------------------------------
bool Shadow_Initialize();
void Shadow_Finalize();

void Shadow_SetMatrices(const ShadowMatrixBuffer& matrices);
void Shadow_SetLightParams(const ShadowLightBuffer& lightParams);
void Shadow_SetLightPosition(const ShadowLightPositionBuffer& lightPos);

void Shadow_Begin(ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* shadowMap);

#endif // SHADOW_H