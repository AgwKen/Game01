/*========================================================================================


   Light Settings [light.cpp]									        PYAE SONE THANT
                                                                        DATE:09/30/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "light.h"
#include "direct3d.h"
using namespace DirectX;


// 定数バッファ
static ID3D11Buffer* g_pPSConstantBuffer1 = nullptr; // b1 - Ambient
static ID3D11Buffer* g_pPSConstantBuffer2 = nullptr; // b2 - Directional
static ID3D11Buffer* g_pPSConstantBuffer3 = nullptr; // b3 - Specular
static ID3D11Buffer* g_pPSConstantBuffer4 = nullptr; // b4 - Point Light

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// ----------------------------------------------------
// 構造体定義
// ----------------------------------------------------
struct DirectionalLight
{
	XMFLOAT4 directional; // ライトの向き（ワールド座標系）
	XMFLOAT4 color;       // ライトの色
};

struct SpecularLight
{
	XMFLOAT3 CameraPosition;
	float    Power;
	XMFLOAT4 Color;
};

struct PointLight
{
	XMFLOAT3 LightPosition;
	float    Range;
	XMFLOAT4 Color;
	/*float SpecularPower;
	XMFLOAT3 SpecularColor;*/
};

struct PointLightList
{
	PointLight light[4];
	int        count;
	XMFLOAT3   dummy;
};

// ----------------------------------------------------
// グローバル変数
// ----------------------------------------------------
static PointLightList g_PointLights{};

// ----------------------------------------------------
// 初期化
// ----------------------------------------------------
void Light_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	// Ambient
	buffer_desc.ByteWidth = sizeof(XMFLOAT4);
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer1);

	// Directional
	buffer_desc.ByteWidth = sizeof(DirectionalLight);
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer2);

	// Specular
	buffer_desc.ByteWidth = sizeof(SpecularLight);
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer3);

	// Point Light
	buffer_desc.ByteWidth = sizeof(PointLightList);
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer4);

	/*
	PointLightList list
	{
		{
			{{0.0f, 2.0f, 0.0f}, 3.0f, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{3.0f, 2.0f, 0.0f}, 3.0f, {0.0f, 0.0f, 1.0f, 1.0f}},
			{{0.0f, 2.0f, 0.0f}, 5.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
			{{0.0f, 2.0f, 0.0f}, 5.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
		},
		2
	};

	g_pContext->UpdateSubresource(g_pPSConstantBuffer4, 0, nullptr, &list, 0, 0);
	g_pContext->PSSetConstantBuffers(4, 1, &g_pPSConstantBuffer4);
	*/
}

// ----------------------------------------------------
// 終了処理
// ----------------------------------------------------
void Light_Finalize()
{
	SAFE_RELEASE(g_pPSConstantBuffer4);
	SAFE_RELEASE(g_pPSConstantBuffer3);
	SAFE_RELEASE(g_pPSConstantBuffer2);
	SAFE_RELEASE(g_pPSConstantBuffer1);
}

// ----------------------------------------------------
// アンビエント設定
// ----------------------------------------------------
void Light_SetAmbient(const XMFLOAT3& color)
{
	g_pContext->UpdateSubresource(g_pPSConstantBuffer1, 0, nullptr, &color, 0, 0);
	g_pContext->PSSetConstantBuffers(1, 1, &g_pPSConstantBuffer1);
}

// ----------------------------------------------------
// ディレクショナルライト設定
// ----------------------------------------------------
void Light_SetDirectionalWorld(const XMFLOAT4& world_directional, const XMFLOAT4& color)
{
	DirectionalLight light{
		world_directional,
		color,
	};

	g_pContext->UpdateSubresource(g_pPSConstantBuffer2, 0, nullptr, &light, 0, 0);
	g_pContext->PSSetConstantBuffers(2, 1, &g_pPSConstantBuffer2);
}

// ----------------------------------------------------
// スペキュラー設定
// ----------------------------------------------------
void Light_SetSpecularWorld(const DirectX::XMFLOAT3& camera_position, float power, const DirectX::XMFLOAT4& color)
{
	SpecularLight light{
		camera_position,
		power,
		color
	};

	g_pContext->UpdateSubresource(g_pPSConstantBuffer3, 0, nullptr, &light, 0, 0);
	g_pContext->PSSetConstantBuffers(3, 1, &g_pPSConstantBuffer3);
}

// ----------------------------------------------------
// ポイントライト数設定
// ----------------------------------------------------
void Light_SetPointLightCount(int count)
{
	g_PointLights.count = count;

	g_pContext->UpdateSubresource(g_pPSConstantBuffer4, 0, nullptr, &g_PointLights, 0, 0);
	g_pContext->PSSetConstantBuffers(4, 1, &g_pPSConstantBuffer4);
}

// ----------------------------------------------------
// 個別ポイントライト設定
// ----------------------------------------------------
void Light_SetPointLight(int n, const DirectX::XMFLOAT3& position, float range, const DirectX::XMFLOAT3& color)
{
	g_PointLights.light[n].LightPosition = position;
	g_PointLights.light[n].Range = range;
	g_PointLights.light[n].Color = { color.x, color.y, color.z, 1.0f };

	g_pContext->UpdateSubresource(g_pPSConstantBuffer4, 0, nullptr, &g_PointLights, 0, 0);
	g_pContext->PSSetConstantBuffers(4, 1, &g_pPSConstantBuffer4);
}