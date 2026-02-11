/*========================================================================================


  Sky cpp [sky.cpp]										            PYAE SONE THANT
                                                                        DATE:11/21/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "sky.h"
#include "cube.h"
#include "camera.h"
#include "player_camera.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "texture.h"
#include "sampler.h"

static int g_SkyTexId = -1;


void Sky_Initialize()
{
    g_SkyTexId = Texture_Load(L"Texture/skybox.png");
}

void Sky_Finalize()
{
}

void Sky_Draw(const XMFLOAT3& camPos)
{
    if (g_SkyTexId < 0) return;

    XMMATRIX world =
        XMMatrixScaling(100.0f, 100.0f, 100.0f) *
        XMMatrixRotationY(XMConvertToRadians(220.0f)) *   // <-- rotate 90 degrees horizontally
        XMMatrixTranslation(camPos.x, camPos.y, camPos.z);

    Sampler_SetFilterAnisotropic();

    Direct3D_SetDepthReadOnly(true);
    Direct3D_SetRasterizerCullFront();

    CUBE_Draw(g_SkyTexId, world);

    Sampler_SetFilterLinear();

    Direct3D_ResetRasterizerState();
    Direct3D_SetDepthReadOnly(false);
}


