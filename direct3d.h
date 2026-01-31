#ifndef DIRECT3D_H
#define DIRECT3D_H

#include <Windows.h>
#include <d3d11.h>

#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }

bool Direct3D_Initialize(HWND hWnd);
void Direct3D_Finalize();
void Direct3D_Clear();
void Direct3D_Present();

unsigned int Direct3D_GetBackBufferWidth();
unsigned int Direct3D_GetBackBufferHeight();

ID3D11Device* Direct3D_GetDevice();
ID3D11DeviceContext* Direct3D_GetDeviceContext();

void Direct3D_SetAlphaBlendState();
void Direct3D_SetAdditiveBlendState();
void Direct3D_SetSubtractiveBlendState();
void Direct3D_SetDefaultBlendState();
void Direct3D_SetMultiplyBlendState();
void Direct3D_SetDepthWriteDisable();
void Direct3D_SetOpaqueBlendState();
void Direct3D_SetDepthEnable(bool enable);
void Direct3D_SetDepthReadOnly(bool enable);

void Direct3D_SetRasterizerCullFront();
void Direct3D_ResetRasterizerState();

extern ID3D11BlendState* g_pAlphaBlendState;
extern ID3D11BlendState* g_pSubtractiveBlendState;
extern ID3D11BlendState* g_pBlendStateMultiply;

void Direct3D_SetOffscreen();

//rendering to offscreen buffer
void Direct3D_SetOffscreenTexture(int slot);
void Direct3D_SetRenderTarget();

HWND Direct3D_GetWindowHandle();

#endif
