#ifndef DIRECT3D_H
#define DIRECT3D_H

#include <Windows.h>
#include <d3d11.h>

// �Z�[�t�����[�X�}�N��
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }

bool Direct3D_Initialize(HWND hWnd);// Direct3D�̏�����
void Direct3D_Finalize();			// Direct3D�̏I������
void Direct3D_Clear();				// �o�b�N�o�b�t�@�̃N���A
void Direct3D_Present();			// �o�b�N�o�b�t�@�̕\��

// Back buffer size
unsigned int Direct3D_GetBackBufferWidth();
unsigned int Direct3D_GetBackBufferHeight();

// Direct3D�f�o�C�X�̎擾
ID3D11Device* Direct3D_GetDevice();

void Direct3D_SetAlphaBlendState();
void Direct3D_SetSubtractiveBlendState();
void Direct3D_SetDefaultBlendState();
void Direct3D_SetMultiplyBlendState();

// Direct3D�f�o�C�X�R���e�L�X�g�̎擾
ID3D11DeviceContext* Direct3D_GetDeviceContext();


// Blend state externs
extern ID3D11BlendState* g_pAlphaBlendState;
extern ID3D11BlendState* g_pSubtractiveBlendState;
extern ID3D11BlendState* g_pBlendStateMultiply;

//����ǂ��o�t�@�ݒ�
void Direct3D_SetDepthEnable(bool enable);

#endif // DIRECT3D_H
