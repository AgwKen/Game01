#include "UI_KickPower.h"
#include "sprite.h"
#include "BallPlayer.h"
#include "direct3d.h"
#include "texture.h"

static float g_DisplayPower = 0.0f;
static float g_LastKickPower = 0.0f;

static float g_ShowTimer = 0.0f;
static constexpr float SHOW_DURATION = 1.2f; // how long bar stays after kick

static int g_WhiteTex = -1;

void UI_KickPower_Initialize()
{
    g_DisplayPower = 0.0f;
    g_LastKickPower = 0.0f;
    g_ShowTimer = 0.0f;

    g_WhiteTex = Texture_Load(L"Texture/white.png");
}

void UI_KickPower_Update(double dt)
{
    float delta = (float)dt;

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

        float normalized = charge / max;

        if (normalized < 0.0f) normalized = 0.0f;
        if (normalized > 1.0f) normalized = 1.0f;

        g_DisplayPower += (normalized - g_DisplayPower) * 12.0f * delta;
    }
    else
    {
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

    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    float barWidth = 300.0f;
    float barHeight = 20.0f;

    float x = (screenW - barWidth) * 0.5f;
    float y = screenH - 80.0f;

    Sprite_Begin();

    // Background
    Sprite_Draw(g_WhiteTex, x, y, barWidth, barHeight,
        { 0.05f, 0.05f, 0.05f, 0.8f });

    float fillWidth = barWidth * g_DisplayPower;

    XMFLOAT4 color;

    if (g_DisplayPower < 0.6f)
        color = { 0.2f, 0.9f, 0.2f, 0.95f };
    else if (g_DisplayPower < 0.9f)
        color = { 0.95f, 0.8f, 0.1f, 1.0f };
    else
        color = { 1.0f, 0.2f, 0.2f, 1.0f };

    Sprite_Draw(g_WhiteTex, x, y, fillWidth, barHeight, color);
}

void UI_KickPower_Finalize()
{
}