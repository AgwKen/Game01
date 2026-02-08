/*==============================================================================

   BillBoard header[billboard.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/
#include "billboard.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader_billboard.h"
#include "texture.h"
#include "player_camera.h"

static constexpr int NUM_VERTEX = 4;
static ID3D11Buffer* g_pVertexBuffer = nullptr;

static XMFLOAT4X4 g_mtxView{};

struct Vertex3d
{
    XMFLOAT3 position;
    XMFLOAT4 color;
    XMFLOAT2 texcoord;
};

void Billboard_Initialize()
{
    ShaderBillBoard_Initialize();

        Vertex3d vertex[]{
            {{-0.5f,  1.0f, 0.0f}, {1,1,1,1}, {0,0}},
            {{ 0.5f,  1.0f, 0.0f}, {1,1,1,1}, {1,0}},
            {{-0.5f, -0.0f, 0.0f}, {1,1,1,1}, {0,1}},
            {{ 0.5f, -0.0f, 0.0f}, {1,1,1,1}, {1,1}},
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertex;

    Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

}

void Billboard_Finalize()
{
    ShaderBillBoard_Finalize();
    SAFE_RELEASE(g_pVertexBuffer);
}

void Billboard_SetViewMatrix(const DirectX::XMFLOAT4X4& view)
{
    g_mtxView = view;
    g_mtxView._41 = g_mtxView._42 = g_mtxView._43 = 0.0f;
}

void Billboard_Draw(
    int texId,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT2& scale,
    const DirectX::XMFLOAT4& tex_cut,
    const DirectX::XMFLOAT2& pivot,
    const DirectX::XMFLOAT4& color /* new parameter */
)
{
    ShaderBillBoard_SetUVParameter({ {1.0f,1.0f},{0.0f,0.0f} });
    ShaderBillBoard_Begin();

    ShaderBillBoard_SetColor(color);

    Texture_SetTexture(texId);

    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;
    Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    XMMATRIX iv = XMMatrixTranspose(XMLoadFloat4x4(&g_mtxView));
    XMMATRIX pivot_offset = XMMatrixTranslation(-pivot.x, -pivot.y, 0.0f);
    XMMATRIX s = XMMatrixScaling(scale.x, scale.y, 1.0f);
    XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
    ShaderBillBoard_SetWorldMatrix(s * pivot_offset * iv * t);

    Direct3D_GetDeviceContext()->Draw(NUM_VERTEX, 0);

}

void Billboard_Draw(
    int texId,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT2& scale,
    const DirectX::XMUINT4& tex_cut,
    const DirectX::XMFLOAT2& pivot,
    const DirectX::XMFLOAT4& color
)
{
    ShaderBillBoard_Begin();

    // 1. Calculate UVs
    float uv_x = (float)tex_cut.x / Texture_Width(texId);
    float uv_y = (float)tex_cut.y / Texture_Height(texId);
    float uv_w = (float)tex_cut.z / Texture_Width(texId);
    float uv_h = (float)tex_cut.w / Texture_Height(texId);

    ShaderBillBoard_SetUVParameter({ {uv_w, uv_h}, {uv_x, uv_y} });
    ShaderBillBoard_SetColor(color);
    Texture_SetTexture(texId);

    // 2. Set Vertex Buffer
    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;
    Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // 3. Prepare the Billboard Matrix (Facing the camera)
    XMMATRIX v = XMLoadFloat4x4(&g_mtxView);

    // This is the trick: We transpose the View Matrix to get the Rotation,
    // but we MUST ensure the translation (row 3) is 0,0,0 so it stays in the world.
    v.r[3] = XMVectorSet(0, 0, 0, 1);
    XMMATRIX iv = XMMatrixTranspose(v);

    // 4. Combine Transformations
    XMMATRIX pivot_offset = XMMatrixTranslation(-pivot.x, -pivot.y, 0.0f);
    XMMATRIX s = XMMatrixScaling(scale.x, scale.y, 1.0f);
    XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);

    // Order: Scale -> Pivot -> Rotation(IV) -> World Position(T)
    ShaderBillBoard_SetWorldMatrix(s * pivot_offset * iv * t);

    // 5. Draw
    Direct3D_GetDeviceContext()->Draw(NUM_VERTEX, 0);
}