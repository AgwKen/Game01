#pragma once

struct RunResult
{
    int goals = 0;
    int coins = 0;
};

void Run_Start(float seconds);
void Run_Update(float dt);
bool Run_IsActive();
bool Run_IsFinished();

float Run_GetTimeLeft();
RunResult Run_GetResult();

void Run_AddGoal();
void Run_AddCoin();

void Run_ResetResultOnly(); // for new run
