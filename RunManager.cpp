#include "RunManager.h"
#include <algorithm>

static bool g_Active = false;
static bool g_Finished = false;
static float g_TimeLeft = 0.0f;
static RunResult g_Result;

void Run_Start(float seconds)
{
    g_Active = true;
    g_Finished = false;
    g_TimeLeft = seconds;
    g_Result = {};
}

void Run_Update(float dt)
{
    if (!g_Active) return;

    g_TimeLeft -= dt;
    if (g_TimeLeft <= 0.0f)
    {
        g_TimeLeft = 0.0f;
        g_Active = false;
        g_Finished = true;
    }
}

bool Run_IsActive() { return g_Active; }
bool Run_IsFinished() { return g_Finished; }

float Run_GetTimeLeft() { return g_TimeLeft; }
RunResult Run_GetResult() { return g_Result; }

void Run_AddGoal() { if (g_Active) g_Result.goals++; }
void Run_AddCoin() { if (g_Active) g_Result.coins++; }

void Run_ResetResultOnly()
{
    g_Result = {};
    g_Finished = false;
}