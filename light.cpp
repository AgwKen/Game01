/*========================================================================================


   Light Settings [light.cpp]									        PYAE SONE THANT
                                                                        DATE:09/30/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "light.h"
using namespace DirectX;

static ID3D11Buffer* g_pVSConstantBuffer3 = nullptr;//b3
static ID3D11Buffer* g_pVSConstantBuffer4 = nullptr;//b4

static ID3D11Device* g_pDevice = nullptr;                   // �������ŊO������ݒ�
static ID3D11DeviceContext* g_pContext = nullptr;           // �������ŊO������ݒ�

struct DirectionalLight
{
	XMFLOAT4 Directional;
	XMFLOAT4 Color;
};

void Light_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// ���_�V�F�[�_�[�p�萔�o�b�t�@�̍쐬
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMFLOAT4);// �o�b�t�@�̃T�C�Y
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;  // �o�C���h�t���O
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer3);//ambient light


	buffer_desc.ByteWidth = sizeof(DirectionalLight);// �o�b�t�@�̃T�C�Y
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer4);//directional light
}

void Light_Finalize()
{
}

void Light_SetAmbientColor(const XMFLOAT4& color)
{
	// �萔�o�b�t�@�A���r�G���g���Z�b�g
	g_pContext->UpdateSubresource(g_pVSConstantBuffer3, 0, nullptr, &color, 0, 0);
	g_pContext->VSSetConstantBuffers(3, 1, &g_pVSConstantBuffer3);
}

void Light_SetDirectionalWorld(const XMFLOAT4& world_directional,const XMFLOAT4& color)
{
	DirectionalLight light{
		world_directional,
		color
	};
	// �萔�o�b�t�@�ɍs����Z�b�g
	g_pContext->UpdateSubresource(g_pVSConstantBuffer4, 0, nullptr, &light, 0, 0);
	g_pContext->VSSetConstantBuffers(4, 1, &g_pVSConstantBuffer4);
}
