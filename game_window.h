/*=======================================================================================
	ゲームウインドウ　GAMEWINDOW [GameWindow.h]				PYAE SONE THANT
																Date:06/06/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HWND GameWindow_Create(HINSTANCE hInstance);

#endif //GAME_WINDOW_H