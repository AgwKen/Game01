/*========================================================================================


	Collision code [collision.cpp]										PYAE SONE THANT
																		DATE:07/03/2025

//------------------------------------------------------------------------------------------

//=========================================================================================*/

#include "collision.h"
#include "direct3d.h"
#include "texture.h"
#include "shader.h"
using namespace DirectX;


static constexpr int NUM_VERTEX = 5000; // ���_��
static ID3D11Buffer* g_pVertexBuffer = nullptr;             // ���_�o�b�t�@
static ID3D11Device* g_pDevice = nullptr;                   // �������ŊO������ݒ�
static ID3D11DeviceContext* g_pContext = nullptr;           // �������ŊO������ݒ�

static int g_WhiteTexId = -1;

bool Collision_IsOverlapCircle(const Circle& a, const Circle& b)
{
	
	float x1 = b.center.x - a.center.x;
	float y1 = b.center.y - a.center.y;

    return (a.radius + b.radius) * (a.radius + b.radius) > (x1 * x1 + y1 * y1);
	

	/*
	XMVECTOR ac = XMLoadFloat2(&a.center);
	XMVECTOR bc = XMLoadFloat2(&b.center);
	XMVECTOR lsq = XMVector2LengthSq(bc - ac);

	return (a.radius + b.radius) * (a.radius + b.radius) > XMVectorGetX(lsq);
	*/
}	

bool Collision_IsOverlapBox(const Box& a, const Box& b)
{
	float at = a.center.y - a.half_height;
	float ab = a.center.y + a.half_height;
	float al = a.center.x - a.half_width;
	float ar = a.center.x + a.half_width;
	float bt = b.center.y - b.half_height;
	float bb = b.center.y + b.half_height;
	float bl = b.center.x - b.half_width;
	float br = b.center.x + b.half_width;

	return al < br && ar > bl && at < bb && ab > bt;
}	

struct Vertex
{
	XMFLOAT3 position; // ���_���W
	XMFLOAT4 color;    // �F
	XMFLOAT2 uv;       // UV(texcoord �e�N�X�`���[���W)
};

void Collision_DebugInitialized(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	
	// �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̕ۑ�
	g_pDevice = pDevice;
	g_pContext = pContext;

	

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);

	g_WhiteTexId = Texture_Load(L"white.png");
}

void Collision_DebugFinalize()
{
	SAFE_RELEASE(g_pVertexBuffer);
}

void Collision_DebugDraw(const Circle& circle ,const  DirectX::XMFLOAT4 color)
{
	int numVertex = (int)(circle.radius * 2.0f * XM_PI + 1);

	// �V�F�[�_�[��`��p�C�v���C���ɐݒ�
	Shader_Begin();

	// ���_�o�b�t�@�����b�N����
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	Vertex* v = (Vertex*)msr.pData;
	// �l�p�`�|���S���̍��W�ݒ�

	const float rad = XM_2PI / numVertex;

	for (int i = 0; i < numVertex;i++) {
		v[i].position.x = cosf(rad * i) * circle.radius + circle.center.x;
		v[i].position.y = sinf(rad * i) * circle.radius + circle.center.y;
		v[i].position.z = 0.0f;
		v[i].color = color;
		v[i].uv = { 0.0f, 0.0f };
	}

	// ���_�o�b�t�@�̃��b�N������
	g_pContext->Unmap(g_pVertexBuffer, 0);

	Shader_SetWorldMatrix(XMMatrixIdentity());

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// �e�N�X�`���ݒ�
	//g_pContext->PSSetShaderResources(0, 1, &g_pTexture);
	Texture_SetTexture(g_WhiteTexId);

	// �|���S���`�施�ߔ��s
	g_pContext->Draw(numVertex, 0);
}

void Collision_DebugDraw(const Box& box, const  DirectX::XMFLOAT4 color)
{
	
	Shader_Begin();

	// ���_�o�b�t�@�����b�N����
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	Vertex* v = (Vertex*)msr.pData;
	// �l�p�`�|���S���̍��W�ݒ�

	// ���_������������
	v[0].position = { box.center.x - box.half_width, box.center.y - box.half_height, 0.0f };
	v[1].position = { box.center.x + box.half_width, box.center.y - box.half_height, 0.0f };
	v[2].position = { box.center.x + box.half_width, box.center.y + box.half_height, 0.0f };
	v[3].position = { box.center.x - box.half_width, box.center.y + box.half_height, 0.0f };
	v[4].position = { box.center.x - box.half_width, box.center.y - box.half_height, 0.0f };
	

	for (int i = 0; i < 5; i++) {
		v[i].color = color;
		v[i].uv = { 0.0f, 0.0f };
	}

	// ���_�o�b�t�@�̃��b�N������
	g_pContext->Unmap(g_pVertexBuffer, 0);

	Shader_SetWorldMatrix(XMMatrixIdentity());

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// �e�N�X�`���ݒ�
	//g_pContext->PSSetShaderResources(0, 1, &g_pTexture);
	Texture_SetTexture(g_WhiteTexId);

	// �|���S���`�施�ߔ��s
	g_pContext->Draw(5, 0);

	
}
