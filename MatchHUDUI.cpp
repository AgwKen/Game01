#include "MatchHUDUI.h"

#include "UIFont.h"
#include "direct3d.h"
#include "texture.h"
#include "sprite.h"

#include <cstdio>
#include <cmath>
#include <cstring>

static UIFont* g_Font = nullptr;
static int g_WhiteTex = -1;
static float g_T = 0.0f;

static void DrawRect(float x, float y, float w, float h, const DirectX::XMFLOAT4& c)
{
    if (g_WhiteTex < 0) return;
    Sprite_Draw(g_WhiteTex, x, y, w, h, 0, 0, 1, 1, c);
}

static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static DirectX::XMFLOAT4 LerpCol(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b, float t)
{
    t = Clamp01(t);
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    };
}

static void DrawOutline(float x, float y, float w, float h, float thick, const DirectX::XMFLOAT4& c)
{
    DrawRect(x, y, w, thick, c);
    DrawRect(x, y + h - thick, w, thick, c);
    DrawRect(x, y, thick, h, c);
    DrawRect(x + w - thick, y, thick, h, c);
}

static void DrawGlowText(const char* text, float x, float y, float scale,
    const DirectX::XMFLOAT4& glowCol,
    const DirectX::XMFLOAT4& mainCol)
{
    if (!g_Font) return;

    float o = 1.2f * scale;
    g_Font->DrawString(text, x - o, y, scale, glowCol);
    g_Font->DrawString(text, x + o, y, scale, glowCol);
    g_Font->DrawString(text, x, y - o, scale, glowCol);
    g_Font->DrawString(text, x, y + o, scale, glowCol);

    g_Font->DrawString(text, x, y, scale, mainCol);
}

// ------------------------------------------------------------
// Text width estimate (fallback)
// If you later add real font measure, replace this.
// ------------------------------------------------------------
static float EstimateTextWidthPx(const char* s, float scale)
{
    // Peaberry is bitmap-ish; average glyph width estimate.
    // 16 px per char at scale=1 is a decent ballpark for many .fnt fonts.
    // If your font feels too tight/loose, tweak baseCharW.
    const float baseCharW = 16.0f;
    int len = (int)strlen(s);
    return (float)len * baseCharW * scale;
}

namespace MatchHUDUI
{
    void Initialize(UIFont* font)
    {
        g_Font = font;
        if (g_WhiteTex < 0)
            g_WhiteTex = Texture_Load(L"Texture/white.png");
        g_T = 0.0f;
    }

    void Finalize()
    {
        g_Font = nullptr;
    }

    void Update(float dt)
    {
        g_T += dt;
    }

    void Draw(float timeLeftSec, int goals, bool runActive, bool runFinished)
    {
        if (!g_Font) return;

        // ------------------------------------------------------------
        // Soccer broadcast palette
        // ------------------------------------------------------------
        DirectX::XMFLOAT4 pitchDark = { 0.05f, 0.16f, 0.08f, 0.88f };
        DirectX::XMFLOAT4 pitchDarker = { 0.03f, 0.11f, 0.06f, 0.88f };
        DirectX::XMFLOAT4 lineWhite = { 0.96f, 0.99f, 0.96f, 0.75f };

        // Animated accent (LED green -> white)
        float mix = (sinf(g_T * 2.4f) + 1.0f) * 0.5f;
        DirectX::XMFLOAT4 ledA = { 0.20f, 0.95f, 0.45f, 1.0f };
        DirectX::XMFLOAT4 ledB = { 0.95f, 0.98f, 0.95f, 1.0f };
        DirectX::XMFLOAT4 led = LerpCol(ledA, ledB, mix);

        // ------------------------------------------------------------
        // Text formatting
        // ------------------------------------------------------------
        int tInt = (int)ceilf(timeLeftSec);
        if (tInt < 0) tInt = 0;

        char timeBuf[64];
        sprintf_s(timeBuf, "TIME %02d", tInt);

        char goalsBuf[64];
        sprintf_s(goalsBuf, "GOALS %d", goals);

        // ------------------------------------------------------------
        // Layout (top-left scoreboard bar) - AUTO SIZE
        // ------------------------------------------------------------
        float x = 26.0f;
        float y = 22.0f;

        float barH = 64.0f;
        float pad = 10.0f;

        float textScale = 2.0f;
        float ty = y + 16.0f; // your current vertical align

        // Minimum box widths (your old values)
        float minTimeBoxW = 170.0f;
        float minGoalsBoxW = 150.0f;

        // Required width based on text
        // add extra padding so glow doesn't touch borders
        float timeNeed = EstimateTextWidthPx(timeBuf, textScale) + 2.0f * (pad + 18.0f);
        float goalsNeed = EstimateTextWidthPx(goalsBuf, textScale) + 2.0f * (pad + 18.0f);

        float timeBoxW = (timeNeed > minTimeBoxW) ? timeNeed : minTimeBoxW;
        float goalsBoxW = (goalsNeed > minGoalsBoxW) ? goalsNeed : minGoalsBoxW;

        // Separator spacing inside bar
        float sepGap = 22.0f;

        float barW = timeBoxW + goalsBoxW + sepGap;

        // Optional: keep it from going insane wide
        // (If it exceeds screen width, it will clip. You can clamp if you want.)
        // float sw = (float)Direct3D_GetBackBufferWidth();
        // if (barW > sw - 20.0f) barW = sw - 20.0f;

        // Drop shadow
        DrawRect(x + 6, y + 7, barW, barH, { 0,0,0,0.28f });

        // Main base
        DrawRect(x, y, barW, barH, pitchDark);
        DrawRect(x + 6, y + 6, barW - 12, barH - 12, pitchDarker);

        // Field-line borders
        DrawOutline(x, y, barW, barH, 2.0f, lineWhite);

        // Accent strip on top
        DrawRect(x, y, barW, 6.0f, { led.x, led.y, led.z, 0.55f });

        // Separator between boxes (now uses timeBoxW)
        float sepX = x + timeBoxW + 10.0f;
        DrawRect(sepX, y + 8, 2.0f, barH - 16.0f, { lineWhite.x, lineWhite.y, lineWhite.z, 0.55f });

        // ------------------------------------------------------------
        // TIME + GOALS text
        // ------------------------------------------------------------
        DirectX::XMFLOAT4 glow = { led.x, led.y, led.z, 0.22f };

        float tx = x + pad;
        DrawGlowText(timeBuf, tx, ty, textScale, glow, { 1,1,1,1 });

        float gx = x + timeBoxW + 22.0f;
        DrawGlowText(goalsBuf, gx, ty, textScale, glow, { 1,1,1,1 });

        // ------------------------------------------------------------
        // Prompts (below bar)
        // ------------------------------------------------------------
        if (!runActive && !runFinished)
        {
            float pulse = 0.55f + 0.45f * ((sinf(g_T * 3.0f) + 1.0f) * 0.5f);
            DirectX::XMFLOAT4 pcol = { led.x, led.y, led.z, 0.65f * pulse };

            float px = x;
            float py = y + barH + 10.0f;
            float pw = 360.0f;
            float ph = 44.0f;

            DrawRect(px + 5, py + 6, pw, ph, { 0,0,0,0.20f });
            DrawRect(px, py, pw, ph, { 0.05f, 0.15f, 0.08f, 0.80f });
            DrawOutline(px, py, pw, ph, 2.0f, { lineWhite.x, lineWhite.y, lineWhite.z, 0.55f });
            DrawRect(px, py, pw, 4.0f, { led.x, led.y, led.z, 0.45f });

            g_Font->DrawString("PRESS SPACE / START", px + 14.0f, py + 10.0f, 1.6f, pcol);
        }

        if (runFinished)
        {
            float px = x;
            float py = y + barH + 10.0f;
            float pw = 220.0f;
            float ph = 44.0f;

            float pulse = 0.35f + 0.35f * ((sinf(g_T * 4.0f) + 1.0f) * 0.5f);
            DirectX::XMFLOAT4 top = { led.x, led.y, led.z, 0.55f * pulse };

            DrawRect(px + 5, py + 6, pw, ph, { 0,0,0,0.20f });
            DrawRect(px, py, pw, ph, { 0.05f, 0.15f, 0.08f, 0.80f });
            DrawOutline(px, py, pw, ph, 2.0f, { lineWhite.x, lineWhite.y, lineWhite.z, 0.55f });
            DrawRect(px, py, pw, 4.0f, top);

            g_Font->DrawString("TIME UP!", px + 18.0f, py + 10.0f, 1.8f, { 1,1,1,1 });

            float hx = px + pw + 10.0f;
            float hw = 340.0f;
            DrawRect(hx + 5, py + 6, hw, ph, { 0,0,0,0.20f });
            DrawRect(hx, py, hw, ph, { 0.05f, 0.15f, 0.08f, 0.72f });
            DrawOutline(hx, py, hw, ph, 2.0f, { lineWhite.x, lineWhite.y, lineWhite.z, 0.45f });
            DrawRect(hx, py, hw, 4.0f, { led.x, led.y, led.z, 0.35f });

            g_Font->DrawString("SPACE/START = NEW RUN", hx + 14.0f, py + 11.0f, 1.5f, { led.x, led.y, led.z, 0.75f });
        }

        // Ågbroadcast dotÅh on the right of bar (moves with new width)
        float dotPulse = 0.20f + 0.20f * ((sinf(g_T * 5.0f) + 1.0f) * 0.5f);
        DrawRect(x + barW - 16.0f, y + 10.0f, 8.0f, 8.0f, { led.x, led.y, led.z, dotPulse + 0.25f });
    }
}