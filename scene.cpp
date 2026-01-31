/*========================================================================================


    Scene cpp code [scene.cpp]					    			    	PYAE SONE THANT
                                                                        DATE:07/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "scene.h"
#include "title.h"
#include "game.h"
#include "fade.h"
#include "result.h"


//static Scene g_Scene = SCENE_TITLE;
static Scene g_Scene = SCENE_GAME;  // starting scene change this
static Scene g_SceneNext = g_Scene;

void Scene_Initialize()
{
    switch (g_Scene)
    {
    case SCENE_TITLE:
        Title_Initialize();
        break;
    case SCENE_GAME:
        Game_Initialize();
        break;
    case SCENE_RESULT:
        Result_Initialize();   // <-- call initialize
        break;
    }
}

void Scene_Finalize()
{
    switch (g_Scene)
    {
    case SCENE_TITLE:
        Title_Finalize();
        break;
    case SCENE_GAME:
        Game_Finalize();
        break;
    case SCENE_RESULT:
        Result_Finalize();
        break;
    }
}

void Scene_Update(double elapsed_time)
{
    switch (g_Scene)
    {
    case SCENE_TITLE:
        Title_Update(elapsed_time);
        break;
    case SCENE_GAME:
        Game_Update(elapsed_time);
        break;
    case SCENE_RESULT:
        Result_Update(elapsed_time);
        break;
    }
}

void Scene_Draw()
{
    switch (g_Scene)
    {
    case SCENE_TITLE:
        Title_Draw();
        break;
    case SCENE_GAME:
        Game_Draw();
        break;
    case SCENE_RESULT:
        Result_Draw();
        break;
    }
}

void Scene_Refresh()
{
    if (g_Scene != g_SceneNext) {
        Scene_Finalize();
        g_Scene = g_SceneNext;
        Scene_Initialize();

        Fade_Start(0.7, false, { 0.0f, 0.0f, 0.0f });
    }
}

void Scene_Change(Scene scene)
{
    g_SceneNext = scene;
}
