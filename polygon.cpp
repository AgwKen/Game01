/*==============================================================================

   �|���S���`�� [polygon.cpp]

                                                         Author : Youhei Sato
                                                         Date   : 2025/05/15

--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "shader.h"
#include "debug_ostream.h"
#include "polygon.h"

#pragma comment(lib, "d3d11.lib")

using namespace DirectX;

//------------------------------------------------------------------------------
// �萔
//------------------------------------------------------------------------------
// NOTE: kept NUM_VERTEX for backward-compatibility but we compute and use g_NumVertex dynamically.
static constexpr int NUM_VERTEX = 4; // ���_�� (not used for allocation anymore)

//------------------------------------------------------------------------------
// �O���[�o���ϐ�
//------------------------------------------------------------------------------
static ID3D11Buffer* g_pVertexBuffer = nullptr;             // ���_�o�b�t�@
static ID3D11ShaderResourceView* g_pTexture = nullptr;      // �e�N�X�`��
static ID3D11Device* g_pDevice = nullptr;                   // �������ŊO������ݒ�
static ID3D11DeviceContext* g_pContext = nullptr;           // �������ŊO������ݒ�
static int g_NumVertex = 0;         // ���ۂɕ`�悷�钸�_�� (���邽�߂� +1 ����ꍇ����)
static float g_Radius = 100.0f;
static float g_Cx = 1000.0f;
static float g_Cy = 500.0f;


//------------------------------------------------------------------------------
// ���_�\����
//------------------------------------------------------------------------------
struct Vertex
{
    XMFLOAT3 position; // ���_���W
    XMFLOAT4 color;    // �F
    XMFLOAT2 uv;       // UV(texcoord �e�N�X�`���[���W)
};

//------------------------------------------------------------------------------
// ������
//------------------------------------------------------------------------------
void Polygon_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    // �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̃`�F�b�N
    if (!pDevice || !pContext) {
        hal::dout << "Polygon_Initialize() : �^����ꂽ�f�o�C�X���R���e�L�X�g���s���ł�" << std::endl;
        return;
    }

    // �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̕ۑ�
    g_pDevice = pDevice;
    g_pContext = pContext;

    // g_Radius �Ɋ�Â��Z�O�����g��������i���Ȃ��Ƃ� 3�j
    // �~�����̃s�N�Z���P�ʋߎ����g���ăZ�O�����g�������߂Ă��܂��B
    int segments = static_cast<int>(g_Radius * 2.0f * XM_PI);
    if (segments < 3) segments = 3;

    // We'll draw a closed line strip: allocate one extra vertex to duplicate the first vertex at the end.
    g_NumVertex = segments + 1;

    // ���_�o�b�t�@�����i���I�ɏ���������z��j
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(Vertex) * g_NumVertex;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    HRESULT hr = g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
    if (FAILED(hr) || !g_pVertexBuffer) {
        hal::dout << "Polygon_Initialize() : ���_�o�b�t�@�쐬���s hr=" << std::hex << hr << std::dec << std::endl;
        return;
    }

    // �e�N�X�`���ǂݍ��݁i�����j
    TexMetadata metadata;
    ScratchImage image;
    HRESULT loadHr = LoadFromWICFile(L"cat.png", WIC_FLAGS_NONE, &metadata, image);
    if (SUCCEEDED(loadHr)) {
        HRESULT createHr = CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_pTexture);
        if (FAILED(createHr)) {
            MessageBox(nullptr, "�e�N�X�`���̓ǂݍ��݂Ɏ��s���� (CreateShaderResourceView)", "�G���[", MB_OK | MB_ICONERROR);
        }
    }
    else {
        MessageBox(nullptr, "�e�N�X�`���̓ǂݍ��݂Ɏ��s���� (LoadFromWICFile)", "�G���[", MB_OK | MB_ICONERROR);
    }
}

//------------------------------------------------------------------------------
// �I������
//------------------------------------------------------------------------------
void Polygon_Finalize(void)
{
    SAFE_RELEASE(g_pTexture);
    SAFE_RELEASE(g_pVertexBuffer);
}

//------------------------------------------------------------------------------
// �`��
//------------------------------------------------------------------------------
void Polygon_Draw(float x, float y, float angle, float radius)
{
    if (!g_pDevice || !g_pContext || !g_pVertexBuffer) {
        return;
    }

    Shader_Begin();

    D3D11_MAPPED_SUBRESOURCE msr;
    HRESULT hr = g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    if (FAILED(hr)) {
        hal::dout << "Polygon_Draw() : VertexBuffer Map failed hr=" << std::hex << hr << std::dec << std::endl;
        return;
    }
    Vertex* v = (Vertex*)msr.pData;

    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    const int segments = 360;
    g_NumVertex = segments * 2;

    // Use the passed-in parameters for the cone's position and size.
    const XMFLOAT3 apex_position = { x, y, 0.0f };
    const XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    const float base_z = 100.0f;

    const float rad = XM_2PI / static_cast<float>(segments);
    for (int i = 0; i < segments; i++) {
        float current_angle = rad * static_cast<float>(i) + angle; // Use `angle` for rotation

        v[i * 2].position = apex_position;
        v[i * 2].color = color;
        v[i * 2].uv = { 0.5f, 0.5f };

        v[i * 2 + 1].position.x = cosf(current_angle) * radius + x;
        v[i * 2 + 1].position.y = sinf(current_angle) * radius + y;
        v[i * 2 + 1].position.z = base_z;
        v[i * 2 + 1].color = color;
        v[i * 2 + 1].uv = { (cosf(current_angle) + 1.0f) * 0.5f, (sinf(current_angle) + 1.0f) * 0.5f };
    }

    g_pContext->Unmap(g_pVertexBuffer, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    Shader_SetProjectionMatrix(XMMatrixPerspectiveFovLH(XM_PIDIV4, SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 1000.0f));

    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    g_pContext->Draw(g_NumVertex, 0);
}