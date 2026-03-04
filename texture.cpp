/*==============================================================================

    //Direct3Dの初期化関連 [direct3d.cpp]
                                                 //Author : Satou Youhei
                                                 //Date    : 2025/06/13
//--------------------------------------------------------------------------------

//==============================================================================*/

#include "texture.h"
#include "direct3d.h"
#include <string>
#include "debug_ostream.h"
#include "WICTextureLoader11.h"
using namespace DirectX;

static constexpr int TEXTURE_MAX = 1024;//texture generator


struct Texture
{
    std::wstring filename;
    unsigned int width;
    unsigned int height;
    ID3D11Resource* pTexture = nullptr;
    ID3D11ShaderResourceView* pTextureView = nullptr;
};

static Texture g_Textures[TEXTURE_MAX]{};
static unsigned int g_SetTextureIndex = -1;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    for (Texture& t : g_Textures)
    {
        t.filename.clear();
        t.width = 0;
        t.height = 0;
        t.pTexture = nullptr;
        t.pTextureView = nullptr;
    }

    g_SetTextureIndex = (unsigned int)-1;
    g_pDevice = pDevice;
    g_pContext = pContext;
}

void Texture_Finalize(void)
{
    Texture_AllRelease();
}

int Texture_Load(const wchar_t* pFilename)
{
    // Already loaded? return same ID
    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        if (!g_Textures[i].filename.empty() && g_Textures[i].filename == pFilename)
            return i;
    }

    // Find free slot
    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        if (g_Textures[i].pTexture) continue;

        // IMPORTANT: device/context must exist
        if (!g_pDevice || !g_pContext)
        {
            MessageBoxW(nullptr, L"Texture_Initialize was not called (device/context is null)", pFilename, MB_OK | MB_ICONERROR);
            return -1;
        }

        HRESULT hr = CreateWICTextureFromFile(
            g_pDevice,
            g_pContext,
            pFilename,
            &g_Textures[i].pTexture,
            &g_Textures[i].pTextureView
        );

        // CHECK FIRST (before GetDesc)
        if (FAILED(hr) || !g_Textures[i].pTexture)
        {
            MessageBoxW(nullptr, L"テクスチャの読み込みに失敗しました", pFilename, MB_OK | MB_ICONERROR);
            g_Textures[i].filename.clear();
            SAFE_RELEASE(g_Textures[i].pTexture);
            SAFE_RELEASE(g_Textures[i].pTextureView);
            return -1;
        }

        // Now safe
        ID3D11Texture2D* pTex2D = (ID3D11Texture2D*)g_Textures[i].pTexture;
        D3D11_TEXTURE2D_DESC desc{};
        pTex2D->GetDesc(&desc);

        g_Textures[i].width = desc.Width;
        g_Textures[i].height = desc.Height;
        g_Textures[i].filename = pFilename;

        return i;
    }

    return -1;
}
void Texture_AllRelease()
{
    for (Texture& t : g_Textures)
    {
        t.filename.clear();
        SAFE_RELEASE(t.pTexture);
        SAFE_RELEASE(t.pTextureView);
        t.width = 0;
        t.height = 0;
    }
}
void Texture_Release(int texid)
{
    if (texid < 0 || texid >= TEXTURE_MAX) return;

    g_Textures[texid].filename.clear();
    SAFE_RELEASE(g_Textures[texid].pTexture);
    SAFE_RELEASE(g_Textures[texid].pTextureView);
    g_Textures[texid].width = 0;
    g_Textures[texid].height = 0;
}

void Texture_SetTexture(int texid, int  slot)
{
    if (texid < 0) return;

    //if (g_SetTextureIndex == texid) return;

    g_SetTextureIndex = texid;

    // テクスチャ設定
    g_pContext->PSSetShaderResources(slot, 1, &g_Textures[texid].pTextureView);
}

unsigned int Texture_Width(int texid)
{
    if (texid < 0) return 0;

    return g_Textures[texid].width;
}

unsigned int Texture_Height(int texid)
{
    if (texid < 0) return 0;

    return g_Textures[texid].height;
}
ID3D11ShaderResourceView* Texture_Get(int texid)
{
    if (texid < 0 || texid >= TEXTURE_MAX) return nullptr;
    return g_Textures[texid].pTextureView;
}

ID3D11ShaderResourceView* Texture_GetSRV(int id)
{
    if (id < 0) return nullptr;
    return Texture_Get(id);
}
