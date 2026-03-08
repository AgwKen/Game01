#include "LeaderboardUI.h"

#include "Leaderboard.h"
#include "RunManager.h"
#include "direct3d.h"
#include "texture.h"
#include "sprite.h"
#include "audio.h"

#include <cstdio>
#include <cmath>

static UIFont* g_Font = nullptr;
static int g_WhiteTex = -1;
static float g_T = 0.0f;

// ============================================================
// RESULT BGM
// ============================================================
static int   g_ResultBGM = -1;
static bool  g_ResultBGMPlaying = false;
static bool  g_WasFinishedLastFrame = false;
static float g_ResultBGMTime = 0.0f;

static constexpr float RESULT_BGM_FULL_VOLUME = 1.0f;
static constexpr float RESULT_BGM_FADE_START = 60.0f; // start fading after 10 sec
static constexpr float RESULT_BGM_FADE_TIME = 2.0f;  // fade length

static bool g_Visible = true;

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

void LeaderboardUI_StopBGM()
{
    if (g_ResultBGMPlaying && g_ResultBGM >= 0)
    {
        StopAudio(g_ResultBGM);                 // <-- must exist in your audio system
        SetAudioVolume(g_ResultBGM, 1.0f);      // reset for next time
    }

    g_ResultBGMPlaying = false;
    g_ResultBGMTime = 0.0f;
}

void LeaderboardUI_Initialize(UIFont* font)
{
    g_Font = font;

    if (g_WhiteTex < 0)
        g_WhiteTex = Texture_Load(L"Texture/white.png");

    if (g_ResultBGM < 0)
        g_ResultBGM = LoadAudio("Sounds/10. World Cup.wav"); // change filename if needed

    g_T = 0.0f;
    g_ResultBGMTime = 0.0f;
    g_ResultBGMPlaying = false;
    g_WasFinishedLastFrame = false;

    g_Visible = true;
}

void LeaderboardUI_Update(float dt)
{
    g_T += dt;

    bool finished = Run_IsFinished();

    // just entered leaderboard/result screen
    if (finished && !g_WasFinishedLastFrame)
    {
        g_Visible = true;
        g_ResultBGMTime = 0.0f;

        if (g_ResultBGM >= 0)
        {
            SetAudioVolume(g_ResultBGM, RESULT_BGM_FULL_VOLUME);
            PlayAudio(g_ResultBGM, true);
            g_ResultBGMPlaying = true;
        }
    }

    // left leaderboard/result screen
    if (!finished && g_WasFinishedLastFrame)
    {
        LeaderboardUI_StopBGM();
    }

    // fade logic while result screen active
    if (finished && g_ResultBGMPlaying && g_ResultBGM >= 0)
    {
        g_ResultBGMTime += dt;

        if (g_ResultBGMTime >= RESULT_BGM_FADE_START)
        {
            float fadeT = (g_ResultBGMTime - RESULT_BGM_FADE_START) / RESULT_BGM_FADE_TIME;
            fadeT = Clamp01(fadeT);

            float vol = RESULT_BGM_FULL_VOLUME * (1.0f - fadeT);
            SetAudioVolume(g_ResultBGM, vol);

            if (fadeT >= 1.0f)
            {
                StopAudio(g_ResultBGM);
                SetAudioVolume(g_ResultBGM, RESULT_BGM_FULL_VOLUME);
                g_ResultBGMPlaying = false;
            }
        }
    }

    g_WasFinishedLastFrame = finished;
}
void LeaderboardUI_Draw()
{
    if (!g_Font) return;
    if (!Run_IsFinished()) return;
    if (!g_Visible) return;

    float sw = (float)Direct3D_GetBackBufferWidth();
    float sh = (float)Direct3D_GetBackBufferHeight();

    // -------------------------
    // SOCCER GREEN THEME COLORS
    // -------------------------
    DirectX::XMFLOAT4 pitchA = { 0.08f, 0.22f, 0.10f, 0.92f };
    DirectX::XMFLOAT4 pitchB = { 0.06f, 0.18f, 0.08f, 0.92f };

    DirectX::XMFLOAT4 lineWhite = { 0.95f, 0.98f, 0.95f, 0.85f };
    DirectX::XMFLOAT4 accentG = { 0.20f, 0.95f, 0.45f, 0.90f };

    float mix = (sinf(g_T * 2.5f) + 1.0f) * 0.5f;
    DirectX::XMFLOAT4 neon = LerpCol(accentG, lineWhite, mix * 0.25f);

    // -------------------------
    // PANEL SIZE (AUTO HEIGHT)
    // -------------------------
    float margin = 24.0f;
    float w = 760.0f;

    int count = Leaderboard_GetCount();
    int rowsWanted = (count < 10) ? count : 10;

    const float hyOffset = 86.0f;
    const float rowStartOffset = 44.0f;
    const float rowStep = 38.0f;
    const float footerStripTop = 50.0f;
    const float bottomPad = 18.0f;
    const float minH = 260.0f;

    const float baseTop = hyOffset + rowStartOffset;
    float needH = baseTop + (rowsWanted * rowStep) + footerStripTop + bottomPad;

    float maxH = sh - margin * 2.0f;
    float h = needH;
    if (h < minH) h = minH;
    if (h > maxH) h = maxH;

    float availableForRows = h - (baseTop + footerStripTop + bottomPad);
    int rowsFit = (int)floorf(availableForRows / rowStep);
    if (rowsFit < 0) rowsFit = 0;

    int rowsToShow = rowsWanted;
    if (rowsToShow > rowsFit) rowsToShow = rowsFit;

    float x = sw - w - margin;
    float y = margin;

    // Background dim
    DrawRect(0, 0, sw, sh, { 0.0f, 0.0f, 0.0f, 0.18f });

    // Shadow
    DrawRect(x + 8, y + 10, w, h, { 0,0,0,0.30f });

    // Panel base
    DrawRect(x, y, w, h, pitchA);
    float stripeW = 42.0f;
    for (float sx = x; sx < x + w; sx += stripeW * 2.0f)
        DrawRect(sx, y, stripeW, h, pitchB);

    // Border
    DrawRect(x - 2, y - 2, w + 4, 2, lineWhite);
    DrawRect(x - 2, y + h, w + 4, 2, lineWhite);
    DrawRect(x - 2, y - 2, 2, h + 4, lineWhite);
    DrawRect(x + w, y - 2, 2, h + 4, lineWhite);

    // Header strip
    DrawRect(x, y, w, 66.0f, { 0.0f, 0.0f, 0.0f, 0.22f });
    DrawRect(x, y + 64.0f, w, 2.0f, { neon.x, neon.y, neon.z, 0.55f });

    // Title
    g_Font->DrawString("RESULT", x + 24, y + 18, 1.8f, { 1,1,1,1 });
    g_Font->DrawString("LEADERBOARD", x + 200, y + 18, 1.8f, { 1,1,1,1 });

    // TIME UP badge
    float badgeW = 130.0f;
    float badgeH = 32.0f;
    float bx = x + w - badgeW - 20.0f;
    float by = y + 18.0f;
    DrawRect(bx, by, badgeW, badgeH, { neon.x, neon.y, neon.z, 0.35f });
    DrawRect(bx, by + badgeH - 2.0f, badgeW, 2.0f, { neon.x, neon.y, neon.z, 0.80f });
    g_Font->DrawString("TIME UP", bx + 18.0f, by + 6.0f, 1.2f, { 1,1,1,1 });

    // Column header
    float hy = y + hyOffset;
    DrawRect(x + 18, hy, w - 36, 36, { 0,0,0,0.28f });

    g_Font->DrawString("RK", x + 34, hy + 7, 1.2f, { 1,1,1,1 });
    g_Font->DrawString("NAME", x + 90, hy + 7, 1.2f, { 1,1,1,1 });
    g_Font->DrawString("SCORE", x + 360, hy + 7, 1.2f, { 1,1,1,1 });
    g_Font->DrawString("G", x + 565, hy + 7, 1.2f, { 1,1,1,1 });
    g_Font->DrawString("C", x + 635, hy + 7, 1.2f, { 1,1,1,1 });

    // Entries
    float rowY = hy + rowStartOffset;
    for (int i = 0; i < rowsToShow; i++)
    {
        const LeaderboardEntry& e = Leaderboard_GetEntry(i);

        float a = (i % 2 == 0) ? 0.18f : 0.10f;
        DrawRect(x + 18, rowY - 2, w - 36, 34, { 0,0,0,a });

        if (i < 3)
        {
            float pulse = 0.08f + 0.10f * ((sinf(g_T * 5.0f + (float)i) + 1.0f) * 0.5f);
            DrawRect(x + 18, rowY - 2, w - 36, 34, { neon.x, neon.y, neon.z, pulse });
        }

        DirectX::XMFLOAT4 rankCol = { 1,1,1,1 };
        if (i == 0) rankCol = { 1.0f, 0.90f, 0.30f, 1.0f };
        if (i == 1) rankCol = { 0.85f, 0.90f, 1.00f, 1.0f };
        if (i == 2) rankCol = { 1.00f, 0.60f, 0.25f, 1.0f };

        DrawRect(x + 340, rowY - 1, 190, 32, { 0.0f, 0.0f, 0.0f, 0.30f });
        DrawRect(x + 340, rowY - 1, 3, 32, { neon.x, neon.y, neon.z, 0.85f });

        char rbuf[8], sbuf[32], gbuf[16], cbuf[16];
        sprintf_s(rbuf, "%d", i + 1);
        sprintf_s(sbuf, "%d", e.score);
        sprintf_s(gbuf, "%d", e.goals);
        sprintf_s(cbuf, "%d", e.coins);

        g_Font->DrawString(rbuf, x + 34, rowY + 2, 1.35f, rankCol);
        g_Font->DrawString(e.name, x + 90, rowY + 2, 1.35f, { 1,1,1,1 });
        g_Font->DrawString(sbuf, x + 368, rowY + 2, 1.35f, { 0.85f, 1.0f, 0.90f, 1.0f });
        g_Font->DrawString(gbuf, x + 565, rowY + 2, 1.35f, { 0.55f, 1.0f, 0.70f, 1.0f });
        g_Font->DrawString(cbuf, x + 635, rowY + 2, 1.35f, { 1.0f, 0.90f, 0.35f, 1.0f });

        rowY += rowStep;
    }

    // Footer
    DrawRect(x + 18, y + h - 50.0f, w - 36, 34.0f, { 0,0,0,0.22f });
    g_Font->DrawString("PRESS SPACE/START = NEW RUN", x + 32, y + h - 44.0f, 1.25f, { 1,1,1,1 });

    // Bottom bar
    float barW = w * 0.30f;
    float barX = x + (w - barW) * 0.5f;
    float barY = y + h - 10.0f;
    float pulse = 0.20f + 0.25f * ((sinf(g_T * 3.2f) + 1.0f) * 0.5f);
    DrawRect(barX, barY, barW, 3.0f, { neon.x, neon.y, neon.z, pulse });
}

void LeaderboardUI_Finalize()
{
    LeaderboardUI_StopBGM();

    if (g_ResultBGM >= 0)
    {
        UnloadAudio(g_ResultBGM);
        g_ResultBGM = -1;
    }
}

void LeaderboardUI_Show()
{
    g_Visible = true;
}

void LeaderboardUI_Hide()
{
    g_Visible = false;
}

void LeaderboardUI_Toggle()
{
    g_Visible = !g_Visible;
}