/*==============================================================================
    Fog System [fog.cpp]
    Author : YOU
==============================================================================*/

#include "fog.h"
#include "direct3d.h"
#include "billboard.h"
#include "shader_billboard.h"
#include "texture.h"

#include <vector>
using namespace DirectX;

// ============================
// Internal Data
// ============================
static std::vector<FogPuff> g_FogList;

static int g_TexFog = -1;        // fog texture ID
static float g_UVScroll = 0.0f;  // UV scroll for animation
static const float SCROLL_SPEED = 0.03f;

// ============================
// Initialization
// ============================
void Fog_Initialize()
{
    g_FogList.reserve(128);

    // Load your fog texture (anime magical fog)
    g_TexFog = Texture_Load(L"Texture/shadow.png");
    // You will replace the path based on your folder

    if (g_TexFog < 0)
        OutputDebugStringA("Fog texture load failed!\n");

    g_FogList.clear();
}

// ============================
// Shutdown
// ============================
void Fog_Finalize()
{
    g_FogList.clear();
}

// ============================
// Spawn a fog puff
// ============================
void Fog_Spawn(const XMFLOAT3& pos, float size, float lifetime)
{
    FogPuff f{};
    f.position = pos;
    f.size = size;
    f.lifetime = lifetime;
    f.totalLifetime = lifetime;
    f.speed = 0.3f + (rand() % 100) * 0.01f; // small random drift
    g_FogList.push_back(f);
}

// ============================
// Update
// ============================
void Fog_Update(double elapsed_time)
{
    // UV scroll
    g_UVScroll += SCROLL_SPEED * (float)elapsed_time;
    if (g_UVScroll > 1.0f) g_UVScroll -= 1.0f;

    // Update puffs
    for (int i = (int)g_FogList.size() - 1; i >= 0; --i)
    {
        FogPuff& f = g_FogList[i];

        // float up slowly
        f.position.y += f.speed * (float)elapsed_time;

        // Fade out and remove
        f.lifetime -= (float)elapsed_time;
        if (f.lifetime <= 0.0f)
        {
            g_FogList.erase(g_FogList.begin() + i);
        }
    }
}

// ============================
// Draw all fog puffs
// ============================
void Fog_Draw()
{
    if (g_FogList.empty()) return;

    ID3D11DeviceContext* ctx = Direct3D_GetDeviceContext();

    // Transparent fog needs:
    Direct3D_SetAlphaBlendState();      // normal alpha blend
    Direct3D_SetDepthReadOnly(true);    // depth test ON, but depth write OFF

    ShaderBillBoard_Begin();
    ShaderBillBoard_SetColor(XMFLOAT4(1, 1, 1, 1));

    // UV scroll values
    UVParameter uv{};
    uv.scale = XMFLOAT2(1.0f, 1.0f);
    uv.translation = XMFLOAT2(g_UVScroll, 0);
    ShaderBillBoard_SetUVParameter(uv);

    for (const FogPuff& f : g_FogList)
    {
        float fade = f.lifetime / f.totalLifetime;

        // Set billboard color (uses shader billboard color)
        ShaderBillBoard_SetColor(XMFLOAT4(1, 1, 1, fade));

        Billboard_Draw(
            g_TexFog,
            f.position,
            XMFLOAT2(f.size, f.size), // scale
            XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), // full texture
            XMFLOAT2(0.5f, 0.5f), // center pivot
            XMFLOAT4(1, 1, 1, fade) // extra color (engine blends both)
        );
    }
    // Reset after drawing
    Direct3D_SetDepthReadOnly(false);
}
