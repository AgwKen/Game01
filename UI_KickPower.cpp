#include "UI_KickPower.h"
#include "sprite.h"
#include "BallPlayer.h"
#include "direct3d.h"
#include "texture.h"

#include <cmath>

static float g_DisplayPower = 0.0f;
static float g_LastKickPower = 0.0f;

static float g_ShowTimer = 0.0f;
static constexpr float SHOW_DURATION = 1.2f; // how long bar stays after kick

static int g_WhiteTex = -1;
static float g_T = 0.0f;

static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static void DrawRect(float x, float y, float w, float h, const XMFLOAT4& c)
{
    if (g_WhiteTex < 0) return;
    Sprite_Draw(g_WhiteTex, x, y, w, h, 0, 0, 1, 1, c);
}

static void DrawOutline(float x, float y, float w, float h, float thick, const XMFLOAT4& c)
{
    DrawRect(x, y, w, thick, c);
    DrawRect(x, y + h - thick, w, thick, c);
    DrawRect(x, y, thick, h, c);
    DrawRect(x + w - thick, y, thick, h, c);
}

static XMFLOAT4 LerpCol(const XMFLOAT4& a, const XMFLOAT4& b, float t)
{
    t = Clamp01(t);
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    };
}

void UI_KickPower_Initialize()
{
    g_DisplayPower = 0.0f;
    g_LastKickPower = 0.0f;
    g_ShowTimer = 0.0f;
    g_T = 0.0f;

    g_WhiteTex = Texture_Load(L"Texture/white.png");
}

void UI_KickPower_Update(double dt)
{
    float delta = (float)dt;
    g_T += delta;

    // If ball reset or no kick active, force hide
    if (!BallPlayer_IsKicked() && !BallPlayer_IsCharging())
    {
        g_DisplayPower = 0.0f;
        g_LastKickPower = 0.0f;
        g_ShowTimer = 0.0f;
        return;
    }

    if (BallPlayer_IsCharging())
    {
        float charge = BallPlayer_GetKickCharge();
        float max = BallPlayer_GetKickMaxPower();
        float normalized = (max > 0.0001f) ? (charge / max) : 0.0f;

        normalized = Clamp01(normalized);

        // smooth follow
        g_DisplayPower += (normalized - g_DisplayPower) * 12.0f * delta;
    }
    else
    {
        // latch last value for a short time after release
        if (g_DisplayPower > 0.01f && g_ShowTimer <= 0.0f)
        {
            g_LastKickPower = g_DisplayPower;
            g_ShowTimer = SHOW_DURATION;
        }

        if (g_ShowTimer > 0.0f)
        {
            g_ShowTimer -= delta;
            g_DisplayPower = g_LastKickPower;
        }
        else
        {
            g_DisplayPower -= 4.0f * delta;
            if (g_DisplayPower < 0.0f)
                g_DisplayPower = 0.0f;
        }
    }
}

void UI_KickPower_Draw()
{
    if (g_DisplayPower <= 0.01f)
        return;

    float sw = (float)Direct3D_GetBackBufferWidth();
    float sh = (float)Direct3D_GetBackBufferHeight();

    // ------------------------------------------------------------
    // Broadcast / soccer HUD style bar (center-bottom)
    // ------------------------------------------------------------
    float barW = 420.0f;
    float barH = 34.0f;
    float x = (sw - barW) * 0.5f;
    float y = sh - 92.0f;

    // colors
    XMFLOAT4 pitchDark = { 0.05f, 0.16f, 0.08f, 0.88f };
    XMFLOAT4 pitchDarker = { 0.03f, 0.11f, 0.06f, 0.88f };
    XMFLOAT4 lineWhite = { 0.96f, 0.99f, 0.96f, 0.75f };

    float mix = (sinf(g_T * 2.4f) + 1.0f) * 0.5f;
    XMFLOAT4 ledA = { 0.20f, 0.95f, 0.45f, 1.0f };
    XMFLOAT4 ledB = { 0.95f, 0.98f, 0.95f, 1.0f };
    XMFLOAT4 led = LerpCol(ledA, ledB, mix);

    // power colors (green -> yellow -> red)
    XMFLOAT4 pLow = { 0.20f, 0.95f, 0.45f, 0.95f };
    XMFLOAT4 pMid = { 0.98f, 0.86f, 0.18f, 1.00f };
    XMFLOAT4 pHigh = { 1.00f, 0.25f, 0.25f, 1.00f };

    float p = Clamp01(g_DisplayPower);
    XMFLOAT4 fillCol;
    if (p < 0.65f)
    {
        fillCol = LerpCol(pLow, pMid, p / 0.65f);
    }
    else
    {
        fillCol = LerpCol(pMid, pHigh, (p - 0.65f) / 0.35f);
    }

    // stronger when "critical"
    float critPulse = 0.0f;
    if (p > 0.90f)
        critPulse = 0.10f + 0.12f * ((sinf(g_T * 12.0f) + 1.0f) * 0.5f);

    // IMPORTANT: Sprite_Begin should already be called in your UI pass.
    // If it is NOT, keep this. If you already call Sprite_Begin() once in RenderPass_UI, you can remove this.
    Sprite_Begin();

    // shadow
    DrawRect(x + 6, y + 7, barW, barH, { 0,0,0,0.28f });

    // base panel
    DrawRect(x, y, barW, barH, pitchDark);
    DrawRect(x + 6, y + 6, barW - 12, barH - 12, pitchDarker);

    // outline
    DrawOutline(x, y, barW, barH, 2.0f, lineWhite);

    // accent strip
    DrawRect(x, y, barW, 5.0f, { led.x, led.y, led.z, 0.55f });

    // inner track
    float innerPad = 10.0f;
    float trackX = x + innerPad;
    float trackY = y + 10.0f;
    float trackW = barW - innerPad * 2.0f;
    float trackH = barH - 18.0f;

    DrawRect(trackX, trackY, trackW, trackH, { 0.0f, 0.0f, 0.0f, 0.32f });
    DrawOutline(trackX, trackY, trackW, trackH, 1.0f, { lineWhite.x, lineWhite.y, lineWhite.z, 0.35f });

    // fill
    float fillW = trackW * p;
    if (fillW < 0.0f) fillW = 0.0f;
    DrawRect(trackX, trackY, fillW, trackH, fillCol);

    // subtle highlight line on fill top
    DrawRect(trackX, trackY, fillW, 2.0f, { 1,1,1,0.12f + critPulse });

    // ticks (like power segments)
    int ticks = 10;
    for (int i = 1; i < ticks; i++)
    {
        float t = (float)i / (float)ticks;
        float lx = trackX + trackW * t;
        DrawRect(lx, trackY + 2.0f, 1.0f, trackH - 4.0f, { lineWhite.x, lineWhite.y, lineWhite.z, 0.10f });
    }

    // right-side glive doth
    float dotPulse = 0.20f + 0.20f * ((sinf(g_T * 5.0f) + 1.0f) * 0.5f);
    DrawRect(x + barW - 14.0f, y + 9.0f, 7.0f, 7.0f, { led.x, led.y, led.z, dotPulse + 0.20f });

    // critical glow overlay
    if (p > 0.90f)
    {
        DrawRect(x, y, barW, barH, { pHigh.x, pHigh.y, pHigh.z, critPulse });
    }
}

void UI_KickPower_Finalize()
{
    // If you have Texture_Unload you can unload g_WhiteTex here.
}