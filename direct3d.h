#ifndef DIRECT3D_H
#define DIRECT3D_H

#include <Windows.h>
#include <d3d11.h>

// セーフリリースマクロ
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }

bool Direct3D_Initialize(HWND hWnd);// Direct3Dの初期化
void Direct3D_Finalize();			// Direct3Dの終了処理
void Direct3D_Clear();				// バックバッファのクリア
void Direct3D_Present();			// バックバッファの表示

// Back buffer size
unsigned int Direct3D_GetBackBufferWidth();
unsigned int Direct3D_GetBackBufferHeight();

// Direct3Dデバイスの取得
ID3D11Device* Direct3D_GetDevice();

void Direct3D_SetAlphaBlendState();
void Direct3D_SetSubtractiveBlendState();
void Direct3D_SetDefaultBlendState();
void Direct3D_SetMultiplyBlendState();

// Direct3Dデバイスコンテキストの取得
ID3D11DeviceContext* Direct3D_GetDeviceContext();


// Blend state externs
extern ID3D11BlendState* g_pAlphaBlendState;
extern ID3D11BlendState* g_pSubtractiveBlendState;
extern ID3D11BlendState* g_pBlendStateMultiply;

//しんどうバファ設定
void Direct3D_SetDepthEnable(bool enable);

#endif // DIRECT3D_H
