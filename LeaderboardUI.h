#pragma once
#include "UIFont.h"

void LeaderboardUI_Initialize(UIFont* font);
void LeaderboardUI_Update(float dt);
void LeaderboardUI_Draw();
void LeaderboardUI_StopBGM();
void LeaderboardUI_Finalize();
void LeaderboardUI_Show();
void LeaderboardUI_Hide();
void LeaderboardUI_Toggle();