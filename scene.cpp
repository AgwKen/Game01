/*========================================================================================


    Scene cpp code [scene.cpp]					    			    	PYAE SONE THANT
                                                                        DATE:07/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "scene.h"
#include "title.h"
#include "game.h"

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
        break;
    }
}

void Scene_Update(double elapsed_time) // increase switch case if u have more scene
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
        break;
    }
}

void Scene_Refresh()
{
    if (g_Scene != g_SceneNext) {
        //present scene finalize å„ï–ïtÇØ

        //next sceneèâä˙å^

        g_Scene = g_SceneNext;

        Scene_Initialize();
    }
}

void Scene_Change(Scene scene)
{
    g_SceneNext = scene;
}
