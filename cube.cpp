    /*========================================================================================

       Cube CPP [cube.cpp]                                               PYAE SONE THANT
                                                                          DATE:09/12/2025
    ------------------------------------------------------------------------------------------

    =========================================================================================*/
    #include "cube.h"
    #include "direct3d.h"
    #include <DirectXMath.h>
    #include "shader3d.h"
    #include "texture.h"

    using namespace DirectX;

    static constexpr int NUM_VERTEX = 24;
    static constexpr int NUM_INDEX = 36;

    static ID3D11Buffer* g_pVertexBuffer = nullptr;
    static ID3D11Buffer* g_pIndexBuffer = nullptr;
    static ID3D11Device* g_pDevice = nullptr;
    static ID3D11DeviceContext* g_pContext = nullptr;

    static int g_CubeTexId = -1;

    struct Vertex3d
    {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT4 color;
        XMFLOAT2 texcoord;
    };

    // 24 vertices for 6 faces
    static const Vertex3d g_CubeVertex[] =
    {
        // Front
        {{-0.5f,  0.5f, -0.5f}, {0, 0, -1}, {1,1,1,1}, {0,0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 0, -1}, {1,1,1,1}, {1,0}},
        {{ 0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1,1,1,1}, {1,1}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1,1,1,1}, {0,1}},

        // Back
        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {0,0}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {1,0}},
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {1,1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {0,1}},

        // Top
        {{-0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1,1,1,1}, {0,0}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1,1,1,1}, {1,0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1,1,1,1}, {1,1}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1,1,1,1}, {0,1}},

        // Bottom
        {{-0.5f, -0.5f, -0.5f}, {0,-1, 0}, {1,1,1,1}, {0,0}},
        {{ 0.5f, -0.5f, -0.5f}, {0,-1, 0}, {1,1,1,1}, {1,0}},
        {{ 0.5f, -0.5f,  0.5f}, {0,-1, 0}, {1,1,1,1}, {1,1}},
        {{-0.5f, -0.5f,  0.5f}, {0,-1, 0}, {1,1,1,1}, {0,1}},

        // Left
        {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1,1,1,1}, {0,0}},
        {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {1,1,1,1}, {1,0}},
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {1,1,1,1}, {1,1}},
        {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1,1,1,1}, {0,1}},

        // Right
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {1,1,1,1}, {0,0}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}, {1,1,1,1}, {1,0}},
        {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}, {1,1,1,1}, {1,1}},
        {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1,1,1,1}, {0,1}},
    };

    static const unsigned short g_CubeIndex[] =
    {
        0, 1, 2, 0, 2, 3,     // Front
        4, 5, 6, 4, 6, 7,     // Back
        8, 9,10, 8,10,11,     // Top
       12,13,14,12,14,15,     // Bottom
       16,17,18,16,18,19,     // Left
       20,21,22,20,22,23      // Right
    };

    void CUBE_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    {
        g_pDevice = pDevice;
        g_pContext = pContext;

        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(g_CubeVertex);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA sd{};
        sd.pSysMem = g_CubeVertex;
        g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

        bd.ByteWidth = sizeof(g_CubeIndex);
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        sd.pSysMem = g_CubeIndex;
        g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);

        g_CubeTexId = Texture_Load(L"Texture/Cass.png");
    }

    void CUBE_Finalize(void)
    {
        SAFE_RELEASE(g_pIndexBuffer);
        SAFE_RELEASE(g_pVertexBuffer);
    }

    void CUBE_Draw(int texId, const DirectX::XMMATRIX& mtxWorld)
    {
        Shader3d_Begin();
        Shader3d_SetColor(XMFLOAT4(1, 1, 1, 1));
        Direct3D_SetDefaultBlendState();

        Texture_SetTexture(texId);

        UINT stride = sizeof(Vertex3d), offset = 0;
        g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        Shader3d_SetWorldMatrix(mtxWorld);
        g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
    }

    AABB Cube_GetAABB(const XMFLOAT3& position)
    {
        return {
            { position.x - 0.5f, position.y - 0.5f, position.z - 0.5f },
            { position.x + 0.5f, position.y + 0.5f, position.z + 0.5f }
        };
    }
