#include "title.h"
#include "texture.h"
#include "sprite.h"
#include "sprite_anim.h"
#include "key_logger.h"
#include "fade.h"
#include "scene.h"
#include "mouse.h"
#include "sampler.h"
#include "game_window.h"

using namespace DirectX;

static int g_BeachTexId = -1;
static int g_ButtonTexId = -1;

static int g_BeachPatternId = -1;
static int g_BeachPlayerId = -1;
static int g_BGFrameTexId = -1;

static int g_StartQuitTexId = -1;
// Button size (on screen)
static const float BTN_W = 200.0f;
static const float BTN_H = 200.0f;

// Button positions
static const float START_X = 560.0f;
static const float START_Y = 250.0f;

static const float QUIT_X = 560.0f;
static const float QUIT_Y = 420.0f;

static const float HOVER_SCALE = 1.08f;


static bool g_HoverStart = false;
static bool g_HoverQuit = false;

static int g_HoverBoxTexId = -1;

const float HOVER_OFFSET_X = -5.0f; // adjust left/right
const float HOVER_OFFSET_Y = 25.0f; // adjust up/down

const float QUIT_OFFSET_X = -5.0f; // adjust left/right
const float QUIT_OFFSET_Y = -40.0f; // adjust up/down

static int g_BackgroundTexId = -1;

static int g_TitleTexId = -1;



void Title_Initialize() {

    g_BeachTexId = Texture_Load(L"Texture/animated-menu.png");
    g_ButtonTexId = Texture_Load(L"Texture/beach-menu-assets.png");
    g_BGFrameTexId = Texture_Load(L"Texture/BGframe.png");
    g_StartQuitTexId = Texture_Load(L"Texture/StartQuit.png");
    g_HoverBoxTexId = Texture_Load(L"Texture/button.png");
    g_BackgroundTexId = Texture_Load(L"Texture/BG.png");
    g_TitleTexId = Texture_Load(L"Texture/title.png");


    g_BeachPatternId = SpriteAnim_RegisterPattern(
        g_BeachTexId, 9, 9, 0.2,
        XMUINT2(96, 96), XMUINT2(0, 0), true
    );

    g_BeachPlayerId = SpriteAnim_CreatePlayer(g_BeachPatternId);

    Fade_Reset();
}

void Title_Finalize() 
{
    SpriteAnim_DestroyPlayer(g_BeachPlayerId);

}

void Title_Update(double elapsed_time)
{
    SpriteAnim_UpdatePlayer(g_BeachPlayerId, elapsed_time);

    Mouse_State mouse;
    Mouse_GetState(&mouse);

    // START hover
    g_HoverStart =
        mouse.x > START_X && mouse.x < START_X + BTN_W &&
        mouse.y > START_Y && mouse.y < START_Y + BTN_H;

    // QUIT hover
    g_HoverQuit =
        mouse.x > QUIT_X && mouse.x < QUIT_X + BTN_W &&
        mouse.y > QUIT_Y && mouse.y < QUIT_Y + BTN_H;

    if (g_HoverStart && mouse.leftButton) {
        FadeState state = Fade_GetState();
        if (state == FADE_STATE_NONE ||
            state == FADE_STATE_FINISHED_IN ||
            state == FADE_STATE_FINISHED_OUT)
        {
            Fade_Start(0.7, true, { 0.0f, 0.0f, 0.0f });
            Scene_Change(SCENE_GAME);
        }
    }




    // Click QUIT
    if (g_HoverQuit && mouse.leftButton) {
        PostQuitMessage(0);
    }
}
void Title_Draw()
{
    Sampler_SetFilterPoint();

    // Get actual window size
    RECT rc;
    GetClientRect(GameWindow_GetHandle(), &rc);
    int winWidth = rc.right - rc.left;
    int winHeight = rc.bottom - rc.top;

    // --- 1. Draw full-screen background ---
    Sprite_Draw(g_BackgroundTexId, 0, 0, (float)winWidth, (float)winHeight);

    // --- 2. Draw title at top center ---
    int titleW = Texture_Width(g_TitleTexId);
    int titleH = Texture_Height(g_TitleTexId);

    // Optional: scale title based on window width
    float titleScale = 1.0f; // adjust this if needed
    float drawX = (winWidth - titleW * titleScale) / 2.0f;
    float drawY = 50.0f; // distance from top

    Sprite_Draw(g_TitleTexId, drawX, drawY, titleW * titleScale, titleH * titleScale);

    // --- 3. Draw animated beach character and frame ---
    SpriteAnim_Draw(g_BeachPlayerId, 420, 160, 480, 480);
    Sprite_Draw(g_BGFrameTexId, 420, 160, 480, 480);

    // --- 4. Draw buttons ---
    int texW = Texture_Width(g_StartQuitTexId);
    int texH = Texture_Height(g_StartQuitTexId);
    int cellW = texW / 2;
    int cellH = texH / 2;

    // START button
    Sprite_Draw(
        g_StartQuitTexId,
        START_X, START_Y,
        BTN_W, BTN_H,
        g_HoverStart ? cellW : 0, 0,
        cellW, cellH
    );

    // QUIT button
    Sprite_Draw(
        g_StartQuitTexId,
        QUIT_X, QUIT_Y,
        BTN_W, BTN_H,
        g_HoverQuit ? cellW : 0, cellH,
        cellW, cellH
    );

    // --- 5. Draw hover boxes ---
    const float BOX_SCALE = 1.3f;

    // START hover box
    if (g_HoverStart) {
        float boxW = BTN_W * BOX_SCALE;
        float boxH = BTN_H * BOX_SCALE;
        float boxX = START_X + (BTN_W - boxW) / 2.0f + HOVER_OFFSET_X;
        float boxY = START_Y + (BTN_H - boxH) / 2.0f + HOVER_OFFSET_Y;
        Sprite_Draw(g_HoverBoxTexId, boxX, boxY, boxW, boxH);
    }

    // QUIT hover box
    if (g_HoverQuit) {
        float boxW = BTN_W * BOX_SCALE;
        float boxH = BTN_H * BOX_SCALE;
        float boxX = QUIT_X + (BTN_W - boxW) / 2.0f + QUIT_OFFSET_X;
        float boxY = QUIT_Y + (BTN_H - boxH) / 2.0f + QUIT_OFFSET_Y;
        Sprite_Draw(g_HoverBoxTexId, boxX, boxY, boxW, boxH);
    }

    Sampler_SetFilterLinear();
}
