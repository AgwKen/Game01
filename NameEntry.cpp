// NameEntry.cpp
#include "NameEntry.h"
#include "Leaderboard.h"
#include "RunManager.h"
#include "UIFont.h"
#include "key_logger.h"
#include "texture.h"
#include "sprite.h"
#include "direct3d.h"

#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <cmath>

// ------------------------------------------------------------
// STATE
// ------------------------------------------------------------
static UIFont* g_FontPtr = nullptr;

static bool g_Active = false;
static RunResult g_Pending{};

static char g_Name[16] = "PLAYER";
static int  g_Len = 6;

static int   g_WhiteTex = -1;

// Screen size cached
static float g_ScreenW = 1280.0f;
static float g_ScreenH = 720.0f;

// Animation
static float g_Time = 0.0f;
static float g_PanelPopT = 1.0f;   // 0..1 on open
static float g_ShakeT = 0.0f;      // shake on invalid
static float g_CursorT = 0.0f;     // blink
static float g_FlashT = 0.0f;      // confirm flash
static float g_StatFlipT = 0.0f;   // stat "shine" animation

// ------------------------------------------------------------
// INTERNAL HELPERS
// ------------------------------------------------------------
static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static float EaseOutBack(float t)
{
    float c1 = 1.70158f;
    float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}

static DirectX::XMFLOAT4 LerpColor(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b, float t)
{
    t = Clamp01(t);
    return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t), Lerp(a.w, b.w, t) };
}

static void ResetName()
{
    strcpy_s(g_Name, "PLAYER");
    g_Len = 6;
}

static void PushChar(char c)
{
    if (g_Len >= 15) return;
    g_Name[g_Len++] = c;
    g_Name[g_Len] = '\0';
}

static void Backspace()
{
    if (g_Len <= 0) return;
    g_Name[--g_Len] = '\0';
}

static bool IsNameValid()
{
    if (g_Len <= 0) return false;
    int good = 0;
    for (int i = 0; i < g_Len; i++)
    {
        char c = g_Name[i];
        if (c != '_' && c != ' ') good++;
    }
    return good > 0;
}

static void DrawRect(float x, float y, float w, float h, const DirectX::XMFLOAT4& color)
{
    if (g_WhiteTex < 0) return;

    Sprite_Draw(
        g_WhiteTex,
        x, y, w, h,
        0, 0, 1, 1,
        color
    );
}

static void DrawGlowText(const char* text, float x, float y, float scale,
    const DirectX::XMFLOAT4& glowCol,
    const DirectX::XMFLOAT4& mainCol)
{
    if (!g_FontPtr) return;

    const float o = 1.6f * scale;

    g_FontPtr->DrawString(text, x - o, y, scale, glowCol);
    g_FontPtr->DrawString(text, x + o, y, scale, glowCol);
    g_FontPtr->DrawString(text, x, y - o, scale, glowCol);
    g_FontPtr->DrawString(text, x, y + o, scale, glowCol);

    g_FontPtr->DrawString(text, x - o, y - o, scale, glowCol);
    g_FontPtr->DrawString(text, x + o, y - o, scale, glowCol);
    g_FontPtr->DrawString(text, x - o, y + o, scale, glowCol);
    g_FontPtr->DrawString(text, x + o, y + o, scale, glowCol);

    g_FontPtr->DrawString(text, x, y, scale, mainCol);
}

// Tiny "chip" label with colored box + text
static void DrawChip(float x, float y, float w, float h,
    const DirectX::XMFLOAT4& boxCol,
    const char* label, const DirectX::XMFLOAT4& labelCol,
    const char* value, const DirectX::XMFLOAT4& valueCol,
    float scale)
{
    DrawRect(x, y, w, h, boxCol);

    // top highlight
    DrawRect(x, y, w, 2.0f * scale, { 1,1,1,0.10f });
    // bottom highlight
    DrawRect(x, y + h - 2.0f * scale, w, 2.0f * scale, { 0,0,0,0.25f });

    float tx = x + 10.0f * scale;
    float ty = y + 8.0f * scale;

    g_FontPtr->DrawString(label, tx, ty, 1.05f * scale, labelCol);

    // value bigger & slightly lower
    g_FontPtr->DrawString(value, tx, ty + 20.0f * scale, 1.55f * scale, valueCol);
}

// ------------------------------------------------------------
// PUBLIC
// ------------------------------------------------------------
void NameEntry_Initialize(UIFont* font)
{
    g_FontPtr = font;

    if (g_WhiteTex < 0)
        g_WhiteTex = Texture_Load(L"Texture/white.png");

    g_ScreenW = (float)Direct3D_GetBackBufferWidth();
    g_ScreenH = (float)Direct3D_GetBackBufferHeight();
}

bool NameEntry_IsActive()
{
    return g_Active;
}

void NameEntry_BeginIfQualifies(const RunResult& r)
{
    if (g_Active) return;
    if (!Leaderboard_WouldEnterTop10(r)) return;

    g_Pending = r;
    g_Active = true;

    ResetName();

    g_ScreenW = (float)Direct3D_GetBackBufferWidth();
    g_ScreenH = (float)Direct3D_GetBackBufferHeight();

    g_Time = 0.0f;
    g_PanelPopT = 0.0f;
    g_ShakeT = 0.0f;
    g_CursorT = 0.0f;
    g_FlashT = 0.0f;
    g_StatFlipT = 0.0f;
}

void NameEntry_Update()
{
    if (!g_Active) return;

    g_Time += 1.0f / 60.0f;
    g_CursorT += 1.0f / 60.0f;
    g_StatFlipT += 1.0f / 60.0f;

    if (g_PanelPopT < 1.0f)
        g_PanelPopT = Clamp01(g_PanelPopT + 0.08f);

    if (g_ShakeT > 0.0f)
        g_ShakeT = Clamp01(g_ShakeT - 0.10f);

    if (g_FlashT > 0.0f)
        g_FlashT = Clamp01(g_FlashT - 0.06f);

    if (KeyLogger_IsTrigger((Keyboard_Keys)VK_BACK))
        Backspace();

    if (KeyLogger_IsTrigger((Keyboard_Keys)VK_RETURN))
    {
        if (IsNameValid())
        {
            Leaderboard_Add(g_Name, g_Pending);
            g_FlashT = 1.0f;
            g_Active = false;
            return;
        }
        else
        {
            g_ShakeT = 1.0f;
        }
    }

    for (int c = 'A'; c <= 'Z'; c++)
    {
        if (KeyLogger_IsTrigger((Keyboard_Keys)c))
        {
            PushChar((char)c);
            break;
        }
    }

    for (int c = '0'; c <= '9'; c++)
    {
        if (KeyLogger_IsTrigger((Keyboard_Keys)c))
        {
            PushChar((char)c);
            break;
        }
    }

    if (KeyLogger_IsTrigger((Keyboard_Keys)VK_SPACE))
        PushChar('_');
}

void NameEntry_Draw()
{
    if (!g_Active) return;
    if (!g_FontPtr) return;

    // ------------------------------------------------------------
    // SOCCER THEME COLORS (pitch green + white lines + LED accent)
    // ------------------------------------------------------------
    const DirectX::XMFLOAT4 cDim = { 0.00f, 0.00f, 0.00f, 0.55f };   // overlay
    const DirectX::XMFLOAT4 cPanel = { 0.07f, 0.18f, 0.09f, 0.90f };   // pitch green
    const DirectX::XMFLOAT4 cPanel2 = { 0.05f, 0.14f, 0.07f, 0.85f };   // darker pitch

    // field line style borders
    const DirectX::XMFLOAT4 cBorder = { 0.94f, 0.98f, 0.94f, 0.35f };
    const DirectX::XMFLOAT4 cBorder2 = { 0.20f, 0.95f, 0.45f, 0.18f }; // green accent

    // animated accent (green -> white)
    const DirectX::XMFLOAT4 cNeonA = { 0.20f, 0.95f, 0.45f, 0.55f };
    const DirectX::XMFLOAT4 cNeonB = { 0.95f, 0.98f, 0.95f, 0.40f };

    const DirectX::XMFLOAT4 cTitleMain = { 1.00f, 1.00f, 1.00f, 1.00f };
    const DirectX::XMFLOAT4 cTitleGlow = { 0.20f, 0.95f, 0.45f, 0.30f };

    const DirectX::XMFLOAT4 cHintBase = { 0.95f, 0.98f, 0.95f, 0.92f };
    const DirectX::XMFLOAT4 cSubtle = { 0.85f, 0.92f, 0.85f, 0.82f };

    // chips
    const DirectX::XMFLOAT4 chipScore = { 0.06f, 0.22f, 0.14f, 0.78f };
    const DirectX::XMFLOAT4 chipGoals = { 0.06f, 0.26f, 0.12f, 0.78f };
    const DirectX::XMFLOAT4 chipCoins = { 0.12f, 0.22f, 0.08f, 0.78f };

    // value colors
    const DirectX::XMFLOAT4 valScoreBase = { 0.60f, 1.00f, 0.72f, 1.00f };
    const DirectX::XMFLOAT4 valGoalsBase = { 0.95f, 0.98f, 0.95f, 1.00f };
    const DirectX::XMFLOAT4 valCoinsBase = { 1.00f, 0.90f, 0.35f, 1.00f };

    // Animated neon
    float neonMix = (sinf(g_Time * 2.2f) + 1.0f) * 0.5f;
    DirectX::XMFLOAT4 neon = LerpColor(cNeonA, cNeonB, neonMix);

    // Background overlay
    DrawRect(0.0f, 0.0f, g_ScreenW, g_ScreenH, cDim);

    float panelW = 720.0f;
    float panelH = 390.0f;

    float pop = EaseOutBack(g_PanelPopT);
    float sc = Lerp(0.75f, 1.0f, pop);

    float panelX = (g_ScreenW - panelW) * 0.5f;
    float panelY = (g_ScreenH - panelH) * 0.5f;

    float cx = panelX + panelW * 0.5f;
    float cy = panelY + panelH * 0.5f;

    float shakeX = 0.0f;
    float shakeY = 0.0f;
    if (g_ShakeT > 0.0f)
    {
        float s = g_ShakeT * g_ShakeT;
        shakeX = sinf(g_Time * 70.0f) * (8.0f * s);
        shakeY = cosf(g_Time * 60.0f) * (5.0f * s);
    }

    float w = panelW * sc;
    float h = panelH * sc;
    float x = cx - w * 0.5f + shakeX;
    float y = cy - h * 0.5f + shakeY;

    // Shadow
    DrawRect(x + 12.0f, y + 16.0f, w, h, { 0,0,0,0.36f });

    // Panel layers
    DrawRect(x, y, w, h, cPanel);
    DrawRect(x + 12.0f, y + 12.0f, w - 24.0f, h - 24.0f, cPanel2);

    // Header strip
    float headerH = 64.0f * sc;
    DrawRect(x, y, w, headerH, { neon.x, neon.y, neon.z, 0.16f });
    DrawRect(x, y + headerH - 2.0f, w, 2.0f, { neon.x, neon.y, neon.z, 0.35f });

    // Borders (field line look)
    DrawRect(x - 2, y - 2, w + 4, 2, cBorder);
    DrawRect(x - 2, y + h, w + 4, 2, cBorder);
    DrawRect(x - 2, y - 2, 2, h + 4, cBorder);
    DrawRect(x + w, y - 2, 2, h + 4, cBorder);

    DrawRect(x - 4, y - 4, w + 8, 2, cBorder2);
    DrawRect(x - 4, y + h + 2, w + 8, 2, cBorder2);
    DrawRect(x - 4, y - 4, 2, h + 8, cBorder2);
    DrawRect(x + w + 2, y - 4, 2, h + 8, cBorder2);

    float tx = x + 40.0f * sc;
    float ty = y + 18.0f * sc;

    DrawGlowText("NEW HIGH SCORE!", tx, ty + 10.0f * sc, 2.35f * sc, cTitleGlow, cTitleMain);

    // -------- STATS (with shine animation) --------
    int score = Leaderboard_ComputeScore(g_Pending);

    float rowFloat = sinf(g_Time * 3.2f) * (3.0f * sc);
    float shine = (sinf(g_StatFlipT * 2.0f) + 1.0f) * 0.5f; // 0..1
    float shineA = Lerp(0.10f, 0.32f, shine);

    char sScore[32], sGoals[32], sCoins[32];
    sprintf_s(sScore, "%d", score);
    sprintf_s(sGoals, "%d", g_Pending.goals);
    sprintf_s(sCoins, "%d", g_Pending.coins);

    float chipY = ty + 86.0f * sc + rowFloat;
    float chipH = 58.0f * sc;
    float gap = 12.0f * sc;
    float chipW = (w - (80.0f * sc) - gap * 2.0f) / 3.0f;

    DirectX::XMFLOAT4 scoreVal = { valScoreBase.x, valScoreBase.y, valScoreBase.z, 1.0f };
    DirectX::XMFLOAT4 goalsVal = { valGoalsBase.x, valGoalsBase.y, valGoalsBase.z, 1.0f };
    DirectX::XMFLOAT4 coinsVal = { valCoinsBase.x, valCoinsBase.y, valCoinsBase.z, 1.0f };

    scoreVal.x = Clamp01(scoreVal.x + shineA * 0.2f);
    scoreVal.y = Clamp01(scoreVal.y + shineA * 0.2f);
    scoreVal.z = Clamp01(scoreVal.z + shineA * 0.2f);

    goalsVal.x = Clamp01(goalsVal.x + shineA * 0.15f);
    goalsVal.y = Clamp01(goalsVal.y + shineA * 0.15f);
    goalsVal.z = Clamp01(goalsVal.z + shineA * 0.15f);

    coinsVal.x = Clamp01(coinsVal.x + shineA * 0.25f);
    coinsVal.y = Clamp01(coinsVal.y + shineA * 0.25f);
    coinsVal.z = Clamp01(coinsVal.z + shineA * 0.25f);

    float chipX0 = tx;
    float chipX1 = chipX0 + chipW + gap;
    float chipX2 = chipX1 + chipW + gap;

    DirectX::XMFLOAT4 chipGlow = { neon.x, neon.y, neon.z, 0.22f + 0.18f * shine };

    DrawChip(chipX0, chipY, chipW, chipH, chipScore, "SCORE", chipGlow, sScore, scoreVal, sc);
    DrawChip(chipX1, chipY, chipW, chipH, chipGoals, "GOALS", chipGlow, sGoals, goalsVal, sc);
    DrawChip(chipX2, chipY, chipW, chipH, chipCoins, "COINS", chipGlow, sCoins, coinsVal, sc);

    // -------- NAME INPUT --------
    g_FontPtr->DrawString("ENTER NAME", tx, ty + 158.0f * sc, 1.55f * sc, cSubtle);

    float boxX = tx;
    float boxY = ty + 198.0f * sc;
    float boxW = w - (80.0f * sc);
    float boxH = 68.0f * sc;

    DrawRect(boxX, boxY, boxW, boxH, { 0.02f, 0.04f, 0.02f, 0.80f });

    float pulse = (sinf(g_Time * 3.5f) + 1.0f) * 0.5f;
    DirectX::XMFLOAT4 outline = { neon.x, neon.y, neon.z, Lerp(0.22f, 0.60f, pulse) };

    DrawRect(boxX - 2, boxY - 2, boxW + 4, 2, outline);
    DrawRect(boxX - 2, boxY + boxH, boxW + 4, 2, outline);
    DrawRect(boxX - 2, boxY - 2, 2, boxH + 4, outline);
    DrawRect(boxX + boxW, boxY - 2, 2, boxH + 4, outline);

    bool cursorOn = fmodf(g_CursorT, 0.9f) < 0.45f;

    char nameBuf[64];
    if (cursorOn) sprintf_s(nameBuf, "%s|", g_Name);
    else          sprintf_s(nameBuf, "%s", g_Name);

    float nameBounce = sinf(g_Time * 6.0f) * (2.4f * sc);

    float lenT = Clamp01((float)g_Len / 12.0f);
    DirectX::XMFLOAT4 nameCol = LerpColor({ 1,1,1,1 }, { neon.x, neon.y, neon.z, 1 }, lenT);

    g_FontPtr->DrawString(nameBuf, boxX + 16.0f * sc, boxY + 14.0f * sc + nameBounce, 2.0f * sc, nameCol);

    // -------- HINTS --------
    float hintPulse = (sinf(g_Time * 2.8f) + 1.0f) * 0.5f;
    DirectX::XMFLOAT4 hintCol = { cHintBase.x, cHintBase.y, cHintBase.z, Lerp(0.65f, 1.0f, hintPulse) };

    float hintY = ty + 288.0f * sc;
    g_FontPtr->DrawString("ENTER = SAVE", tx, hintY, 1.25f * sc, hintCol);
    g_FontPtr->DrawString("BACKSPACE = DELETE", tx + 240.0f * sc, hintY, 1.25f * sc, { 0.90f, 0.98f, 0.90f, 0.85f });

    // Decorative bottom bars
    float barY = y + h - 22.0f * sc;
    float barW = w * 0.36f;
    float barX = x + (w - barW) * 0.5f;

    DrawRect(barX, barY, barW, 3.0f * sc, { neon.x, neon.y, neon.z, 0.35f });
    DrawRect(barX + 18.0f * sc, barY + 6.0f * sc, barW - 36.0f * sc, 2.0f * sc, { neon.x, neon.y, neon.z, 0.25f });
}