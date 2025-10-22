/*========================================================================================

 ゲームウインドウ  GAMEWINDOW <GameWindow.h>                     PYAE SONE THANT
                                                                DATE:06/06/2005

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "game_window.h"
#include <algorithm>
#include "keyboard.h"
#include "mouse.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


static constexpr char WINDOW_CLASS[] = "GameWindow";
static constexpr char TITLE[] = "Game";

static const constexpr int SCREEN_WIDTH = 1023;  // Windowed mode size
static const constexpr int SCREEN_HEIGHT = 576;


HWND GameWindow_Create(HINSTANCE hInstance)
{
    WNDCLASSEX wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WINDOW_CLASS;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    int desktop_width = GetSystemMetrics(SM_CXSCREEN);
    int desktop_height = GetSystemMetrics(SM_CYSCREEN);

    HWND hWnd;

    // =====================================================
    // Toggle between FULLSCREEN and WINDOWED by commenting
    // =====================================================

    // --- FULLSCREEN MODE ---
//     DWORD style = WS_POPUP;  // No title bar, no borders
 //    hWnd = CreateWindow(
   //      WINDOW_CLASS,
     //    TITLE,
       //  style,
       //  0,
       //  0,
       //  desktop_width,
       //  desktop_height,
       //  nullptr, nullptr, hInstance, nullptr);

  //   --- WINDOWED MODE ---
    DWORD style = WS_OVERLAPPEDWINDOW;  // Standard window with borders
    hWnd = CreateWindow(
        WINDOW_CLASS,
        TITLE,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        SCREEN_WIDTH,
       SCREEN_HEIGHT,
       nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);

    return hWnd;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ACTIVATEAPP:
        Keyboard_ProcessMessage(message, wParam, lParam);
        Mouse_ProcessMessage(message, wParam, lParam);
        break;

    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse_ProcessMessage(message, wParam, lParam);
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
        }
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        Keyboard_ProcessMessage(message, wParam, lParam);
        break;

    case WM_CLOSE:
        if (MessageBox(hWnd, "Are you sure you want to close?", "Confirm Exit", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK) {
            DestroyWindow(hWnd);
        }
        // If NO is selected, do nothing (prevents closing)
        break;
    case WM_DESTROY:// Triggers WM_DESTROY
        PostQuitMessage(0);

        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);

    }
    return 0;
}
