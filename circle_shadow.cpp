/*==============================================================================

  Circle Shadow[circle_shadow.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/12/12
--------------------------------------------------------------------------------

==============================================================================*/
#include "circle_shadow.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader3d_unlit.h"
#include "texture.h"
#include "player_camera.h"
#include "collision.h"
#include "map.h"

static constexpr int NUM_VERTEX = 4;
static ID3D11Buffer* g_pVertexBuffer = nullptr;

static int g_TexId{ -1 };

struct Vertex3d
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT4 color;
    XMFLOAT2 texcoord;
};

void CircleShadow_Initialize()
{
    Vertex3d vertex[]{
        {{-0.5f,0.0f, 0.5f }, {0.0f, 1.0f, 0.0f},  {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,0.0f ,0.5f  }, {0.0f, 1.0f, 0.0f},  {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f,0.0f ,-0.5f },{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f,0.0f ,-0.5f },{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertex;

    Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

    g_TexId = Texture_Load(L"Texture/shadow2.png");
}

void CircleShadow_Finalize()
{
    SAFE_RELEASE(g_pVertexBuffer);
}

void CircleShadow_Draw(const DirectX::XMFLOAT3& position)
{
    float y = -1000.0f;
    AABB shadow_aabb{ {position.x - 0.5f, position.y - 10.0f, position.z - 0.5f},
       {position.x + 0.5f, position.y + 10.0f, position.z + 0.5f } };

    for (int i = 0; i < Map_GetObjectCount(); i++) {

        // ここがオブジェクトによって変わるはず
        AABB object = Map_GetObject(i)->Aabb;

        bool isHit = Collision_IsOverlapAABB(object, shadow_aabb);

        if (isHit) {
            if (y < object.max.y) {
                y = object.max.y;
			}
        }
    }

    Shader3dUnlit_Begin();

    Shader3dUnlit_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    //shadow texture
    Texture_SetTexture(g_TexId);

    UINT stride = sizeof(Vertex3d);
    UINT offset = 0;
    Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    Shader3dUnlit_SetWorldMatrix(
        XMMatrixScaling(2.0f, 2.0f, 2.0f) *
        XMMatrixTranslation(position.x, y + 0.01f, position.z));

    Direct3D_GetDeviceContext()->Draw(NUM_VERTEX, 0);

}
