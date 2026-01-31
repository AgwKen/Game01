#include "result.h"
#include "scene.h"
#include "fade.h"
#include "texture.h"
#include "game_window.h"
#include "sprite.h"
#include "sampler.h"

// Result screen variables
static int g_ResultTexId = -1;
static double g_ResultTimer = 0.0;

void Result_Initialize()
{
    // Load the result background texture
    g_ResultTexId = Texture_Load(L"Texture/Win.png");
    g_ResultTimer = 0.0;

    // Start fade-in effect
    Fade_Start(0.7, false, { 0.0f, 0.0f, 0.0f });
}

void Result_Finalize()
{
    // Release texture
    if (g_ResultTexId != -1) {
        Texture_Release(g_ResultTexId);
        g_ResultTexId = -1;
    }
    Fade_Reset();
}

void Result_Update(double elapsed_time)
{
    g_ResultTimer += elapsed_time;

    // After 5 seconds, return to title
    if (g_ResultTimer >= 5.0) {
        Scene_Change(SCENE_TITLE);
    }
}

void Result_Draw()
{
    if (g_ResultTexId == -1) return;

    Sampler_SetFilterPoint();

    // Get window size
    RECT rc;
    GetClientRect(GameWindow_GetHandle(), &rc);
    int winWidth = rc.right - rc.left;
    int winHeight = rc.bottom - rc.top;

    // Draw full-screen background
    Sprite_Draw(g_ResultTexId, 0.0f, 0.0f, (float)winWidth, (float)winHeight);

    Sampler_SetFilterLinear();
}
