/*========================================================================================


    Audio Header code [Audio.h]												PYAE SONE THANT
                                                                        DATE:07/09/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#pragma once

void InitAudio();
void UninitAudio();


int LoadAudio(const char* FileName);
void UnloadAudio(int Index);
void PlayAudio(int Index, bool Loop = false);

