/*========================================================================================


   Light Settings [light.cpp]									        PYAE SONE THANT
                                                                        DATE:09/30/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "light.h"
using namespace DirectX;

static ID3D11Buffer* g_pVSConstantBuffer3 = nullptr;//b3
static ID3D11Buffer* g_pVSConstantBuffer4 = nullptr;//b4

static ID3D11Device* g_pDevice = nullptr;                   // 初期化で外部から設定
static ID3D11DeviceContext* g_pContext = nullptr;           // 初期化で外部から設定

struct DirectionalLight
{
	XMFLOAT4 Directional;
	XMFLOAT4 Color;
};

void Light_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMFLOAT4);// バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;  // バインドフラグ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer3);//ambient light


	buffer_desc.ByteWidth = sizeof(DirectionalLight);// バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer4);//directional light
}

void Light_Finalize()
{
}

void Light_SetAmbientColor(const XMFLOAT4& color)
{
	// 定数バッファアンビエントをセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer3, 0, nullptr, &color, 0, 0);
	g_pContext->VSSetConstantBuffers(3, 1, &g_pVSConstantBuffer3);
}

void Light_SetDirectionalWorld(const XMFLOAT4& world_directional,const XMFLOAT4& color)
{
	DirectionalLight light{
		world_directional,
		color
	};
	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer4, 0, nullptr, &light, 0, 0);
	g_pContext->VSSetConstantBuffers(4, 1, &g_pVSConstantBuffer4);
}
