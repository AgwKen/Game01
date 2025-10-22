/*========================================================================================


    Score Header code [score.h]												PYAE SONE THANT
                                                                            DATE:07/09/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef SCORE_H
#define SCORE_H

void Score_Initialize(float x,float y,int digit); //Other options include zero padding, left justification, etc.
void Score_Finalize();
void Score_Update();
void Score_Draw();

unsigned int  Score_GetScore();
void Score_AddScore(int score);
void Score_Reset();


#endif // SCORE_H

