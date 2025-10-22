/*==============================================================================

   �X�v���C�g�`�� [sprite.cpp]
   Author : Youhei Sato
   Date   : 2025/06/13

==============================================================================*/

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "shader.h"
#include "debug_ostream.h"
#include "sprite.h"
#include "texture.h"

#pragma comment(lib, "d3d11.lib")

using namespace DirectX;

//==============================================================================
// �萔 / �O���[�o���ϐ�
//==============================================================================
static constexpr int NUM_VERTEX = 4; // ���_��

static ID3D11Buffer* g_pVertexBuffer = nullptr;           // ���_�o�b�t�@
static ID3D11ShaderResourceView* g_pTexture = nullptr;    // �e�N�X�`��
static ID3D11Device* g_pDevice = nullptr;                 // �f�o�C�X
static ID3D11DeviceContext* g_pContext = nullptr;         // �R���e�L�X�g

//==============================================================================
// ���_�\����
//==============================================================================
struct Vertex {
    XMFLOAT3 position; // ���_���W
    XMFLOAT4 color;    // �F
    XMFLOAT2 uv;       // �e�N�X�`�����W
};

//==============================================================================
// ������ / ���
//==============================================================================
void Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {
    if (!pDevice || !pContext) {
        hal::dout << "Sprite_Initialize() : �^����ꂽ�f�o�C�X���R���e�L�X�g���s���ł�" << std::endl;
        return;
    }
    g_pDevice = pDevice;
    g_pContext = pContext;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    g_pDevice->CreateBuffer(&bd, nullptr, &g_pVertexBuffer);
}

void Sprite_Finalize() {
    SAFE_RELEASE(g_pTexture);
    SAFE_RELEASE(g_pVertexBuffer);
}

//==============================================================================
// �`��O����
//==============================================================================
void Sprite_Begin() {
    const float SCREEN_WIDTH = static_cast<float>(Direct3D_GetBackBufferWidth());
    const float SCREEN_HEIGHT = static_cast<float>(Direct3D_GetBackBufferHeight());

    Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
}

//==============================================================================
// �`��֐��n (�I�[�o�[���[�h)
//==============================================================================

// �T�C�Y�w��Ȃ�
void Sprite_Draw(int texid, float dx, float dy, const XMFLOAT4& color) {
    Sprite_Draw(texid, dx, dy, static_cast<float>(Texture_Width(texid)), static_cast<float>(Texture_Height(texid)), color);
}

// �w��͈͂̕`��
void Sprite_Draw(int texid, float dx, float dy, float dw, float dh, const XMFLOAT4& color) {
    D3D11_MAPPED_SUBRESOURCE msr;
    g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    Vertex* v = static_cast<Vertex*>(msr.pData);

    v[0].position = { dx,      dy,      0.0f };
    v[1].position = { dx + dw, dy,      0.0f };
    v[2].position = { dx,      dy + dh, 0.0f };
    v[3].position = { dx + dw, dy + dh, 0.0f };

    for (int i = 0; i < 4; i++) v[i].color = color;

    v[0].uv = { 0.0f, 0.0f };
    v[1].uv = { 1.0f, 0.0f };
    v[2].uv = { 0.0f, 1.0f };
    v[3].uv = { 1.0f, 1.0f };

    g_pContext->Unmap(g_pVertexBuffer, 0);

    Shader_SetWorldMatrix(XMMatrixIdentity());
    Shader_Begin();

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    Texture_SetTexture(texid);
    g_pContext->Draw(NUM_VERTEX, 0);
}

// UV�w��`��
void Sprite_Draw(int texid, float dx, float dy, int px, int py, int pw, int ph, const XMFLOAT4& color) {
    Sprite_Draw(texid, dx, dy, static_cast<float>(pw), static_cast<float>(ph), px, py, pw, ph, color);
}

void Sprite_Draw(int texid, float dx, float dy, float dw, float dh, int px, int py, int pw, int ph, const XMFLOAT4& color) {
    D3D11_MAPPED_SUBRESOURCE msr;
    g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    Vertex* v = static_cast<Vertex*>(msr.pData);

    v[0].position = { dx,      dy,      0.0f };
    v[1].position = { dx + dw, dy,      0.0f };
    v[2].position = { dx,      dy + dh, 0.0f };
    v[3].position = { dx + dw, dy + dh, 0.0f };

    for (int i = 0; i < 4; i++) v[i].color = color;

    float tw = static_cast<float>(Texture_Width(texid));
    float th = static_cast<float>(Texture_Height(texid));

    float u0 = px / tw;
    float v0 = py / th;
    float u1 = (px + pw) / tw;
    float v1 = (py + ph) / th;

  //fliiping system
    if (dw < 0) {
        float u_temp = u0;
        u0 = u1;
        u1 = u_temp;
        dw = -dw;
    }

    v[0].uv = { u0, v0 };
    v[1].uv = { u1, v0 };
    v[2].uv = { u0, v1 };
    v[3].uv = { u1, v1 };

    g_pContext->Unmap(g_pVertexBuffer, 0);

    Shader_SetWorldMatrix(XMMatrixIdentity());
    Shader_Begin();

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    Texture_SetTexture(texid);
    g_pContext->Draw(NUM_VERTEX, 0);
}

// ��]�t���`��
void Sprite_Draw(int texid, float dx, float dy, float dw, float dh, int px, int py, int pw, int ph, float angle, const XMFLOAT4& color) {
    D3D11_MAPPED_SUBRESOURCE msr;
    g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    Vertex* v = static_cast<Vertex*>(msr.pData);

    v[0].position = { -0.5f, -0.5f, 0.0f };
    v[1].position = { 0.5f, -0.5f, 0.0f };
    v[2].position = { -0.5f,  0.5f, 0.0f };
    v[3].position = { 0.5f,  0.5f, 0.0f };

    for (int i = 0; i < 4; i++) v[i].color = color;

    float tw = static_cast<float>(Texture_Width(texid));
    float th = static_cast<float>(Texture_Height(texid));

    float u0 = px / tw;
    float v0 = py / th;
    float u1 = (px + pw) / tw;
    float v1 = (py + ph) / th;

    //fliping  system
    if (dw < 0) {
        float u_temp = u0;
        u0 = u1;
        u1 = u_temp;
        dw = -dw;
    }

    v[0].uv = { u0, v0 };
    v[1].uv = { u1, v0 };
    v[2].uv = { u0, v1 };
    v[3].uv = { u1, v1 };

    g_pContext->Unmap(g_pVertexBuffer, 0);

    XMMATRIX scale = XMMatrixScaling(dw, dh, 1.0f);
    XMMATRIX rotation = XMMatrixRotationZ(angle);
    XMMATRIX translation = XMMatrixTranslation(dx + dw * 0.5f, dy + dh * 0.5f, 0.0f);

    Shader_SetWorldMatrix(scale * rotation * translation);
    Shader_Begin();

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    Texture_SetTexture(texid);
    g_pContext->Draw(NUM_VERTEX, 0);
}