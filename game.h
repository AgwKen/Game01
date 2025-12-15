/*========================================================================================


    Main Game [game.h]									    			PYAE SONE THANT
                                                                        DATE:06/27/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef GAME_H
#define GAME_H

void Game_Initialize();
void Game_Finalize();

void Game_Update(double elapsed_time);
void Game_Draw();
void Game_RequestHitStop(float time);
bool Game_IsHitStopActive();


#endif // GAME_H
