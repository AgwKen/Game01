#pragma once
#include "UIFont.h"

void LeaderboardUI_Initialize(UIFont* font);
void LeaderboardUI_Update(float dt);
void LeaderboardUI_Draw();   // draws only when Run_IsFinished()