#include "UI_GoalAnim.h"
#include "texture.h"
#include "sprite.h"
#include "direct3d.h"
#include <cmath>

// ============================================================
// SETTINGS
// ============================================================

static const wchar_t* GOAL_TEX_PATH = L"Texture/goal.png";

// final size on screen
static constexpr float GOAL_W = 600.0f;
static constexpr float GOAL_H = 300.0f;

// timing
static constexpr float FADE_IN_TIME = 0.2f;  // slower appear
static constexpr float HOLD_TIME = 1.5f;  // stay longer
static constexpr float FADE_OUT_TIME = 0.2f;  // slower disappear

static constexpr float TOTAL_TIME = FADE_IN_TIME + HOLD_TIME + FADE_OUT_TIME;

// slide offsets (pixels)
static constexpr float SLIDE_IN_OFFSET = -250.0f;
static constexpr float SLIDE_OUT_OFFSET = 300.0f;

// small pop scale
static constexpr float POP_SCALE = 1.0f;

// ============================================================
// STATE
// ============================================================

static int   g_goalTex = -1;
static bool  g_active = false;
static float g_timer = 0.0f;

// ============================================================
// HELPERS
// ============================================================

static float EaseOut(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
static float EaseIn(float t) { return t * t; }

// ============================================================
// INIT / FINALIZE
// ============================================================

void UI_GoalAnim_Initialize()
{
    g_goalTex = Texture_Load(GOAL_TEX_PATH);

    // If load failed, keep inactive (prevents crashes)
    g_active = false;
    g_timer = 0.0f;
}

void UI_GoalAnim_Finalize()
{
    // IMPORTANT:
    // Do NOT Texture_Release() here unless your engine truly supports per-texture release safely.
    // Many student engines cache textures and free everything in Texture_AllRelease() at shutdown.
    g_goalTex = -1;
    g_active = false;
    g_timer = 0.0f;
}

// ============================================================
// PLAY / UPDATE
// ============================================================

void UI_GoalAnim_Play()
{
    if (g_goalTex < 0) return; // texture not loaded
    g_active = true;
    g_timer = 0.0f;
}

void UI_GoalAnim_Update(double dt)
{
    if (!g_active) return;

    g_timer += (float)dt;

    if (g_timer >= TOTAL_TIME)
        g_active = false;
}

// ============================================================
// DRAW
// ============================================================

void UI_GoalAnim_Draw()
{
    if (!g_active || g_goalTex < 0) return;

    float alpha = 1.0f;
    float offsetX = 0.0f;
    float scale = 1.0f;

    // FADE IN + SLIDE IN
    if (g_timer < FADE_IN_TIME)
    {
        float t = g_timer / FADE_IN_TIME;
        float e = EaseOut(t);

        alpha = e;
        offsetX = SLIDE_IN_OFFSET * (1.0f - e);
        scale = 1.0f + (POP_SCALE - 1.0f) * (1.0f - e);
    }
    // HOLD
    else if (g_timer < FADE_IN_TIME + HOLD_TIME)
    {
        alpha = 1.0f;
        offsetX = 0.0f;
        scale = 1.0f;
    }
    // FADE OUT + SLIDE OUT
    else
    {
        float t = (g_timer - FADE_IN_TIME - HOLD_TIME) / FADE_OUT_TIME;
        float e = EaseIn(t);

        alpha = 1.0f - e;
        offsetX = SLIDE_OUT_OFFSET * e;
        scale = 1.0f;
    }

    // Center on screen (top-left coordinate)
    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    float drawW = GOAL_W * scale;
    float drawH = GOAL_H * scale;

    float baseX = (screenW - drawW) * 0.5f;
    float baseY = (screenH - drawH) * 0.22f; // slightly above center looks nicer

    float drawX = baseX + offsetX;
    float drawY = baseY;

    // IMPORTANT: source rect must be valid (NOT 0,0,0,0)
    int srcW = (int)Texture_Width(g_goalTex);
    int srcH = (int)Texture_Height(g_goalTex);
    if (srcW <= 0 || srcH <= 0) return;
    drawX = floorf(drawX + 0.5f);
    drawY = floorf(drawY + 0.5f);
    drawW = floorf(drawW + 0.5f);
    drawH = floorf(drawH + 0.5f);

    Sprite_Draw(
        g_goalTex,
        drawX, drawY,
        drawW, drawH,
        0, 0,
        (float)srcW, (float)srcH,
        { 1, 1, 1, alpha }
    );
}