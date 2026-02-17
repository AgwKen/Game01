#pragma once

#include <d3d11.h>

bool Shadow_Initialize();
void Shadow_Finalize();

ID3D11DepthStencilView* Shadow_GetDSV();
ID3D11ShaderResourceView* Shadow_GetShadowMap();

void Shadow_SetRenderTarget();
void Shadow_Clear();
ID3D11SamplerState* Shadow_GetSampler();
