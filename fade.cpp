/*========================================================================================

    Fade Cpp code [fade.cpp]                                             PYAE SONE THANT
                                                                            DATE:07/10/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "fade.h"
using namespace DirectX;
#include <algorithm>
#include "texture.h"
#include "sprite.h"
#include "direct3d.h"

// Internal fade state variables
static float g_FadeTime = 0.0f;
static float g_FadeStartTime = 0.0f;
static float g_AccumulatedTime = 0.0f;
static XMFLOAT3 g_Color{ 0.0f, 0.0f, 0.0f };
static float g_Alpha = 0.0f;
static FadeState g_State = FADE_STATE_NONE;
static int g_FadeTexId = -1;

void Fade_Initialize()
{
    g_FadeTime = 0.0f;
    g_FadeStartTime = 0.0f;
    g_AccumulatedTime = 0.0f;
    g_Color = { 0.0f, 0.0f, 0.0f };
    g_Alpha = 0.0f;
    g_State = FADE_STATE_NONE;

  g_FadeTexId = Texture_Load(L"Texture/white.png");
}

void Fade_Finalize()
{
    // Optional cleanup if needed
}

void Fade_Update(double elapsed_time)
{
    if (g_State != FADE_STATE_IN && g_State != FADE_STATE_OUT) return;

    // Convert double -> float safely
    g_AccumulatedTime += static_cast<float>(elapsed_time);

    float ratio = std::min((g_AccumulatedTime - g_FadeStartTime) / g_FadeTime, 1.0f);
    g_Alpha = (g_State == FADE_STATE_IN) ? (1.0f - ratio) : ratio;

    if (ratio >= 1.0f) {
        g_State = (g_State == FADE_STATE_IN) ? FADE_STATE_FINISHED_IN : FADE_STATE_FINISHED_OUT;
    }
}

void Fade_Draw()
{
    if (g_State == FADE_STATE_NONE || g_State == FADE_STATE_FINISHED_IN) return;

    XMFLOAT4 color{ g_Color.x, g_Color.y, g_Color.z, g_Alpha };
    Sprite_Draw(
        g_FadeTexId,
        0.0f,
        0.0f,
        static_cast<float>(Direct3D_GetBackBufferWidth()),
        static_cast<float>(Direct3D_GetBackBufferHeight()),
        color
    );
}

void Fade_Start(double time, bool isFadeOut, DirectX::XMFLOAT3 color)
{
    g_FadeStartTime = g_AccumulatedTime;
    g_FadeTime = static_cast<float>(time);
    g_State = isFadeOut ? FADE_STATE_OUT : FADE_STATE_IN;
    g_Color = color;
    g_Alpha = isFadeOut ? 0.0f : 1.0f;
}

FadeState Fade_GetState()
{
    return g_State;
}
