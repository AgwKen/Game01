/*========================================================================================


    MAIN CPP												PYAE SONE THANT
                                                            DATE:06/06/2005

------------------------------------------------------------------------------------------

=========================================================================================*/
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "game_window.h"
#include "direct3d.h"
#include "shader.h"
#include "shader3d.h"
#include "sampler.h"
#include "sprite.h"
#include "texture.h"
#include "sprite_anim.h"
#include "fade.h"
#include"collision.h"
#include "debug_text.h"
#include <sstream>
#include "system_timer.h"
#include <math.h>
#include <DirectXMath.h>
using namespace DirectX;
#include  "key_logger.h"
#include "mouse.h"
#include "scene.h"
#include "Audio.h"
#include "cube.h"
#include "Grid.h"
#include "light.h"


//#include <Xinput.h>
//#pragma comment(lib,"xinput.lib")

/*-------------------------------------------------------------------------------
    ���C��
�E-------------------------------------------------------------------------------*/

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
   (void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);

   // DPI�X�P�[�����O
   //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HWND hWnd = GameWindow_Create(hInstance);

    SystemTimer_Initialize();
    KeyLogger_Initialize();
    Mouse_Initialize(hWnd);
    InitAudio();

    Direct3D_Initialize(hWnd);
    Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    Shader3d_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
	Sampler_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    Texture_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    Sprite_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    SpriteAnim_Initialize();
    Fade_Initialize();
    Mouse_SetVisible(true);
    Scene_Initialize();
	Grid_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
	Light_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
    CUBE_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
	//MeshField_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

#if defined(DEBUG) || defined(_DEBUG)

    hal::DebugText dt(Direct3D_GetDevice(), Direct3D_GetDeviceContext(),
        L"Texture/consolab_ascii_512.png", Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight(),
        0.0f, 0.0f,
        0, 0,
        0.0f, 16.0f);

    Collision_DebugInitialized(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

#endif

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    //Frame setting ���s�t���C��
    double exec_last_time = SystemTimer_GetTime();
    double fps_last_time = exec_last_time;
    double current_time = 0.0;
    ULONG frame_count = 0;
    double fps = 0.0;

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            current_time = SystemTimer_GetTime();//�V�X�e���������擾
            double elapsed_time = current_time - fps_last_time; //fps�v���p�̌o�ߎ��Ԃ��v�Z

            if (elapsed_time >= 1.0)//1�b���ƂɌv��
            {
                fps = frame_count / elapsed_time;
                fps_last_time = current_time;//FPS�𑪒肵��������ۑ�
                frame_count = 0;//�J�E���g���N���A
            }

            // 1/60�b���ƂɎ��s
            elapsed_time = current_time - exec_last_time;
            if (elapsed_time >= (1.0 / 60.0)) {
                // Update last execution time
                exec_last_time = current_time;

                KeyLogger_Update(); 
              //  Mouse_State ms{};
              //  Mouse_GetState(&ms);

                Scene_Update(elapsed_time);

                SpriteAnim_Update(elapsed_time);
                Fade_Update(elapsed_time);


                // Clear the screen
                Direct3D_Clear();
                Sprite_Begin();

                  Scene_Draw();
                  Fade_Draw();

            // Draw FPS counter
#if defined(DEBUG) || defined(_DEBUG)
                  std::stringstream ss;
                  ss << "FPS: " << fps << std::endl;
                  dt.SetText(ss.str().c_str());
                  dt.Draw();
                  dt.Clear();
#endif


                // Present frame
                Direct3D_Present();

                Scene_Refresh();

                // Increment frame count
                frame_count++;
            }
        }
    }

#if defined(DEBUG) || defined(_DEBUG)
    Collision_DebugFinalize();
#endif
	Grid_Finalize();
    CUBE_Finalize();
	Light_Finalize();
    Scene_Finalize();
    Fade_Finalize();
    SpriteAnim_Finalize();
    Sprite_Finalize();
    Texture_Finalize();
	Sampler_Finalize();
    Shader_Finalize();
    Shader3d_Finalize();
    Direct3D_Finalize();
    UninitAudio();
    Mouse_Finalize();

    CoUninitialize();
    return (int)msg.wParam;
}
