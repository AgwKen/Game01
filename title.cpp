/*========================================================================================


    title cpp code [title.cpp]												PYAE SONE THANT
                                                                            DATE:07/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "title.h"
#include "texture.h"
#include "sprite.h"
#include "key_logger.h"
#include "fade.h"
#include "scene.h"


static int g_TitleBgTexId = -1;


void Title_Initialize()
{
    g_TitleBgTexId = Texture_Load(L"Texture/TitleScreen.png");
}

void Title_Finalize()
{
   // Texture_AllRelease();
}

void Title_Update(double elapsed_time)
{
    if (KeyLogger_IsTrigger(KK_ENTER)) {
        Fade_Start(1.0f, true);
    }
    if (Fade_GetState() == FADE_STATE_FINISHED_OUT) {
        Scene_Change(SCENE_GAME);
    }
}


void Title_Draw()
{
    Sprite_Draw(g_TitleBgTexId, 0.0f, 0.0f,1400, 800);
}
