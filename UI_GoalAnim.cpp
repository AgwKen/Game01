#include "UI_GoalAnim.h"
#include "sprite_anim.h"
#include "texture.h"

static int g_pattern = -1;
static int g_player = -1;
static bool g_active = false;

void UI_GoalAnim_Initialize()
{
    int tex = Texture_Load(L"Texture/goal_sprite.png");

    g_pattern = SpriteAnim_RegisterPattern(
        tex,
        16,        // total frames
        4,         // horizontal frames
        0.2,      // seconds per frame
        { 256,230 }, // frame size
        { 0,0 },
        false      // play once
    );
}

void UI_GoalAnim_Play()
{
    g_player = SpriteAnim_CreatePlayer(g_pattern);
    g_active = true;
}

void UI_GoalAnim_Update(double dt)
{
    if (!g_active) return;

    SpriteAnim_UpdatePlayer(g_player, dt);

    if (SpriteAnim_IsStopped(g_player))
    {
        g_active = false;
        SpriteAnim_DestroyPlayer(g_player);
        g_player = -1;
    }
}

void UI_GoalAnim_Draw()
{
    if (!g_active) return;

    SpriteAnim_Draw(g_player, 600, 200, 600, 300); // center screen
}