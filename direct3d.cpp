/*==============================================================================

Direct3Dの初期化関連 [direct3d.cpp]
Author : PYAE SONE THANT
Date   : 2025/05/12
-------------------

==============================================================================*/
#include <d3d11.h>
#include "direct3d.h"
#include "debug_ostream.h"
#include <DirectXMath.h>
using namespace DirectX;
#pragma comment(lib, "d3d11.lib")

/* 各種インターフェース */
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11BlendState* g_pBlendStateMultiply = nullptr;
static ID3D11BlendState* g_pAdditiveBlendState = nullptr; // added
static ID3D11DepthStencilState* g_pDepthStencilStateDepthDisable = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilStateDepthEnable = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilStateDepthReadOnly = nullptr;
static ID3D11SamplerState* g_pPointSampler = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilStateDepthWriteDisable = nullptr;

/* バックバッファ関連 */
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
static ID3D11Texture2D* g_pDepthStencilBuffer = nullptr;
static ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
static D3D11_TEXTURE2D_DESC g_BackBufferDesc{};
static D3D11_VIEWPORT g_Viewport{};

static bool configureBackBuffer();
static void releaseBackBuffer();

static ID3D11BlendState* g_pAlphaBlendState = nullptr;
static ID3D11BlendState* g_pSubtractiveBlendState = nullptr;
static ID3D11BlendState* g_pOpaqueBlendState = nullptr;

//offscreen render target
static ID3D11Texture2D* g_pOffscreenBuffer = nullptr;
static ID3D11RenderTargetView* g_pOffscreenRenderTargetView= nullptr;
static ID3D11ShaderResourceView* g_pOffscreenShaderResourceView = nullptr;
static ID3D11Texture2D* g_pOffscreenDepthStencilBuffer = nullptr;
static ID3D11DepthStencilView* g_pOffscreenDepthStencilView = nullptr;
static D3D11_TEXTURE2D_DESC g_OffscreenDesc{};
static D3D11_VIEWPORT g_OffscreenViewport{};

static bool configureOffscreenBuffer();
static void releaseOffscreenBuffer();

bool Direct3D_Initialize(HWND hWnd)
{
	/* デバイス、スワップチェーン、コンテキスト生成 */
	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
	swap_chain_desc.Windowed = TRUE;
	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swap_chain_desc.OutputWindow = hWnd;

		UINT device_flags = 0;

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		device_flags,
		levels,
		ARRAYSIZE(levels),
		D3D11_SDK_VERSION,
		&swap_chain_desc,
		&g_pSwapChain,
		&g_pDevice,
		&feature_level,
		&g_pDeviceContext);

	if (FAILED(hr)) {
		MessageBox(hWnd, "Direct3Dの初期化に失敗しました", "エラー", MB_OK);
		return false;
	}

	if (!configureBackBuffer()) {
		MessageBox(hWnd, "バックバッファの設定に失敗しました", "エラー", MB_OK);
		return false;
	}

	//offscreen buffer
	configureOffscreenBuffer();

	/* ===========================
	   Blend State Settings
	   =========================== */

	D3D11_BLEND_DESC bd = {};
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	g_pDevice->CreateBlendState(&bd, &g_pBlendStateMultiply);
	float blend_factor[4] = { 0,0,0,0 };
	g_pDeviceContext->OMSetBlendState(g_pBlendStateMultiply, blend_factor, 0xFFFFFFFF);

	// Additive Blend State
	D3D11_BLEND_DESC additiveBlendDesc = {};
	additiveBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	additiveBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	additiveBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	additiveBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	additiveBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	additiveBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_pDevice->CreateBlendState(&additiveBlendDesc, &g_pAdditiveBlendState);

	/* ===========================
	   Depth Stencil States
	   =========================== */

	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = FALSE;

	// Depth Disabled
	dsd.DepthEnable = FALSE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	g_pDevice->CreateDepthStencilState(&dsd, &g_pDepthStencilStateDepthDisable);

	// Depth Enable
	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	g_pDevice->CreateDepthStencilState(&dsd, &g_pDepthStencilStateDepthEnable);

	// Depth Test ON, Depth Write OFF (transparent objects)
	D3D11_DEPTH_STENCIL_DESC transparentDepthDesc = {};
	transparentDepthDesc.DepthEnable = TRUE;
	transparentDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	transparentDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	transparentDepthDesc.StencilEnable = FALSE;
	g_pDevice->CreateDepthStencilState(&transparentDepthDesc, &g_pDepthStencilStateDepthReadOnly);

	/* DepthWriteDisable (full disable) */
	D3D11_DEPTH_STENCIL_DESC depthWriteDisableDesc = {};
	depthWriteDisableDesc.DepthEnable = FALSE;
	depthWriteDisableDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthWriteDisableDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthWriteDisableDesc.StencilEnable = FALSE;
	g_pDevice->CreateDepthStencilState(&depthWriteDisableDesc, &g_pDepthStencilStateDepthWriteDisable);

	Direct3D_SetDepthEnable(true);

	/* Alpha, subtractive, opaque blends */
	D3D11_BLEND_DESC alphaBlendDesc = {};
	alphaBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	alphaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	alphaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alphaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	alphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_pDevice->CreateBlendState(&alphaBlendDesc, &g_pAlphaBlendState);

	D3D11_BLEND_DESC subtractiveBlendDesc = {};
	subtractiveBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	subtractiveBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	subtractiveBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	subtractiveBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	subtractiveBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	subtractiveBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	subtractiveBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	subtractiveBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_pDevice->CreateBlendState(&subtractiveBlendDesc, &g_pSubtractiveBlendState);

	D3D11_BLEND_DESC opaqueBlendDesc = {};
	opaqueBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	opaqueBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_pDevice->CreateBlendState(&opaqueBlendDesc, &g_pOpaqueBlendState);

	/* ===========================
	Point Sampler (no blur)
   =========================== */
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = g_pDevice->CreateSamplerState(&sampDesc, &g_pPointSampler);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "Point sampler failed", "Error", MB_OK);
		return false;
	}

	return true;

}

void Direct3D_Finalize()
{
	SAFE_RELEASE(g_pDepthStencilStateDepthDisable);
	SAFE_RELEASE(g_pDepthStencilStateDepthEnable);
	SAFE_RELEASE(g_pDepthStencilStateDepthReadOnly);
	SAFE_RELEASE(g_pDepthStencilStateDepthWriteDisable);

	SAFE_RELEASE(g_pBlendStateMultiply);
	SAFE_RELEASE(g_pAdditiveBlendState);
	SAFE_RELEASE(g_pAlphaBlendState);
	SAFE_RELEASE(g_pSubtractiveBlendState);
	SAFE_RELEASE(g_pOpaqueBlendState);
	SAFE_RELEASE(g_pPointSampler);

	releaseBackBuffer();
	releaseOffscreenBuffer();

	SAFE_RELEASE(g_pSwapChain);
	SAFE_RELEASE(g_pDeviceContext);
	SAFE_RELEASE(g_pDevice);

}

void Direct3D_Clear()
{
	g_pDeviceContext->RSSetViewports(1, &g_Viewport);

	g_pDeviceContext->PSSetSamplers(0, 1, &g_pPointSampler);

	float clear_color[4] = { 0.5f, 0.7f, 0.8f, 1.0f };
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clear_color);
	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

}

void Direct3D_Present()
{
	g_pSwapChain->Present(1, 0);
}

unsigned int Direct3D_GetBackBufferWidth() { return g_BackBufferDesc.Width; }
unsigned int Direct3D_GetBackBufferHeight() { return g_BackBufferDesc.Height; }
ID3D11Device* Direct3D_GetDevice() { return g_pDevice; }
ID3D11DeviceContext* Direct3D_GetDeviceContext() { return g_pDeviceContext; }

void Direct3D_SetAlphaBlendState()
{
	float blend_factor[4] = { 0,0,0,0 };
	g_pDeviceContext->OMSetBlendState(g_pAlphaBlendState, blend_factor, 0xFFFFFFFF);
}

void Direct3D_SetAdditiveBlendState()
{
	float blend_factor[4] = { 0,0,0,0 };
	g_pDeviceContext->OMSetBlendState(g_pAdditiveBlendState, blend_factor, 0xFFFFFFFF);
}

void Direct3D_SetSubtractiveBlendState()
{
	float blend_factor[4] = { 0,0,0,0 };
	g_pDeviceContext->OMSetBlendState(g_pSubtractiveBlendState, blend_factor, 0xFFFFFFFF);
}

void Direct3D_SetOpaqueBlendState()
{
	g_pDeviceContext->OMSetBlendState(g_pOpaqueBlendState, nullptr, 0xFFFFFFFF);
}

void Direct3D_SetDefaultBlendState()
{
	g_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void Direct3D_SetMultiplyBlendState()
{
	float blend_factor[4] = { 0,0,0,0 };
	g_pDeviceContext->OMSetBlendState(g_pBlendStateMultiply, blend_factor, 0xFFFFFFFF);
}

void Direct3D_SetDepthEnable(bool enable)
{
	if (enable)
		g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateDepthEnable, 0);
	else
		g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateDepthDisable, 0);
}

void Direct3D_SetDepthReadOnly(bool enable)
{
	if (enable)
		g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateDepthReadOnly, 0);
	else
		g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateDepthEnable, 0);
}
void Direct3D_SetOffscreen()
{
	g_pDeviceContext->RSSetViewports(1, &g_OffscreenViewport);

	float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	g_pDeviceContext->ClearRenderTargetView(g_pOffscreenRenderTargetView, clear_color);
	g_pDeviceContext->ClearDepthStencilView(g_pOffscreenDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	g_pDeviceContext->OMSetRenderTargets(1, &g_pOffscreenRenderTargetView, g_pOffscreenDepthStencilView);

}

void Direct3D_SetOffscreenTexture(int slot)
{
	g_pDeviceContext->PSSetShaderResources(slot, 1, &g_pOffscreenShaderResourceView);
}

void Direct3D_SetDepthWriteDisable()
{
	g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateDepthWriteDisable, 0);
}

bool configureBackBuffer()
{
	ID3D11Texture2D* back_buffer_pointer = nullptr;

		HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
			(void**)&back_buffer_pointer);

	if (FAILED(hr)) {
		hal::dout << "バックバッファの取得に失敗しました" << std::endl;
		return false;
	}

	hr = g_pDevice->CreateRenderTargetView(back_buffer_pointer, nullptr, &g_pRenderTargetView);
	if (FAILED(hr)) {
		back_buffer_pointer->Release();
		hal::dout << "バックバッファのRTV生成失敗" << std::endl;
		return false;
	}

	back_buffer_pointer->GetDesc(&g_BackBufferDesc);
	back_buffer_pointer->Release();

	D3D11_TEXTURE2D_DESC depth_desc{};
	depth_desc.Width = g_BackBufferDesc.Width;
	depth_desc.Height = g_BackBufferDesc.Height;
	depth_desc.MipLevels = 1;
	depth_desc.ArraySize = 1;
	depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_desc.SampleDesc.Count = 1;
	depth_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = g_pDevice->CreateTexture2D(&depth_desc, nullptr, &g_pDepthStencilBuffer);
	if (FAILED(hr)) return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
	dsv_desc.Format = depth_desc.Format;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	hr = g_pDevice->CreateDepthStencilView(
		g_pDepthStencilBuffer, &dsv_desc, &g_pDepthStencilView);

	if (FAILED(hr)) return false;

	g_Viewport.TopLeftX = 0;
	g_Viewport.TopLeftY = 0;
	g_Viewport.Width = (FLOAT)g_BackBufferDesc.Width;
	g_Viewport.Height = (FLOAT)g_BackBufferDesc.Height;
	g_Viewport.MinDepth = 0.0f;
	g_Viewport.MaxDepth = 1.0f;

	g_pDeviceContext->RSSetViewports(1, &g_Viewport);
	return true;

}

void releaseBackBuffer()
{
	SAFE_RELEASE(g_pRenderTargetView);
	SAFE_RELEASE(g_pDepthStencilBuffer);
	SAFE_RELEASE(g_pDepthStencilView);

	SAFE_RELEASE(g_pDepthStencilStateDepthReadOnly);

}

bool configureOffscreenBuffer()
{
	HRESULT hr;

	g_OffscreenDesc.Width = 512;
	g_OffscreenDesc.Height = 512;
	g_OffscreenDesc.MipLevels = 1;
	g_OffscreenDesc.ArraySize = 1;
	g_OffscreenDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	g_OffscreenDesc.SampleDesc.Count = 1;
	g_OffscreenDesc.SampleDesc.Quality = 0;
	g_OffscreenDesc.Usage = D3D11_USAGE_DEFAULT;
	g_OffscreenDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	g_OffscreenDesc.CPUAccessFlags = 0;
	g_OffscreenDesc.MiscFlags = 0;

	hr = g_pDevice->CreateTexture2D(&g_OffscreenDesc, nullptr, &g_pOffscreenBuffer);
	if (FAILED(hr) || !g_pOffscreenBuffer) return false;

	hr = g_pDevice->CreateRenderTargetView(g_pOffscreenBuffer, nullptr, &g_pOffscreenRenderTargetView);
	if (FAILED(hr)) return false;

	hr = g_pDevice->CreateShaderResourceView(g_pOffscreenBuffer, nullptr, &g_pOffscreenShaderResourceView);
	if (FAILED(hr)) return false;

	// depth stencil
	D3D11_TEXTURE2D_DESC depth_stencil_desc{};
	depth_stencil_desc.Width = g_OffscreenDesc.Width;
	depth_stencil_desc.Height = g_OffscreenDesc.Height;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.ArraySize = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_desc.SampleDesc.Count = 1;
	depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = g_pDevice->CreateTexture2D(&depth_stencil_desc, nullptr, &g_pOffscreenDepthStencilBuffer);
	if (FAILED(hr) || !g_pOffscreenDepthStencilBuffer) return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = depth_stencil_desc.Format;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;

	hr = g_pDevice->CreateDepthStencilView(
		g_pOffscreenDepthStencilBuffer,
		&depth_stencil_view_desc,
		&g_pOffscreenDepthStencilView
	);
	if (FAILED(hr)) return false;

	g_OffscreenViewport.TopLeftX = 0.0f;
	g_OffscreenViewport.TopLeftY = 0.0f;
	g_OffscreenViewport.Width = static_cast<FLOAT>(g_OffscreenDesc.Width);
	g_OffscreenViewport.Height = static_cast<FLOAT>(g_OffscreenDesc.Height);
	g_OffscreenViewport.MinDepth = 0.0f;
	g_OffscreenViewport.MaxDepth = 1.0f;

	return true;
}

void releaseOffscreenBuffer()
{
	SAFE_RELEASE(g_pOffscreenBuffer);
	SAFE_RELEASE(g_pOffscreenRenderTargetView);
	SAFE_RELEASE(g_pOffscreenShaderResourceView);
	SAFE_RELEASE(g_pOffscreenDepthStencilBuffer);
	SAFE_RELEASE(g_pOffscreenDepthStencilView);
}
