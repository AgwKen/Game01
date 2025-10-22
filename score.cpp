/*========================================================================================


    Score Header cpp [score.cpp]												PYAE SONE THANT
                                                                                DATE:07/09/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "score.h"
#include "texture.h"
#include "sprite.h"
#include "algorithm"

static unsigned int  g_Score = 0;
static unsigned int g_ViewScore = 0;

static int g_Digit = 1;
static unsigned int g_CounterStop = 1;
static float g_x = 0.0f, g_y = 0.0f;
static int  g_ScoreTexId = -1;
static constexpr int SCORE_FONT_WIDTH = 500;
static constexpr int SCORE_FONT_HEIGHT = 600;

static void  drawNumber(float x, float y, int number);

void Score_Initialize(float x, float y, int digit)
{
    g_Score = 0;
    g_ViewScore = 0;
    g_Digit = digit;
    g_x = x;
    g_y = y;

    //counter stop
    for (int i = 0; i < digit; i++) {
        g_CounterStop *= 10;
    }

    g_CounterStop--;

    g_ScoreTexId = Texture_Load(L"ScoreNum.png");

}

void Score_Finalize()
{
}

void Score_Update()
{
    g_ViewScore = std::min(g_ViewScore + 1, g_Score);
}

void Score_Draw()
{
    unsigned int temp = std::min(g_ViewScore,g_CounterStop);

    for (int i = 0; i < g_Digit; i++) {

        int n = g_Score % 10;

        float x = g_x + (g_Digit - 1 - i) * SCORE_FONT_WIDTH;
        drawNumber(x, g_y, n);

        temp /= 10;
    }
}

unsigned int Score_GetScore()
{
	return g_Score;
}

void Score_AddScore(int score)
{
    g_ViewScore = g_Score;
    g_Score += score;
}

void Score_Reset()
{
    g_Score = 0;
}

void drawNumber(float x, float y, int number)
{
    Sprite_Draw(g_ScoreTexId, x, y, SCORE_FONT_WIDTH * number, 0, SCORE_FONT_WIDTH, SCORE_FONT_HEIGHT);
}
