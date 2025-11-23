/*========================================================================================


  SkyDome Header [sky.h]										        PYAE SONE THANT
                                                                        DATE:11/21/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "sky.h"
using namespace DirectX;
#include "model.h"
#include "shader3d_unlit.h"
#include "direct3d.h"

static MODEL* g_pModelSky{ nullptr };
static XMFLOAT3 g_Position{};

void Sky_Initialize()
{
    g_pModelSky = ModelLoad("Resources/Model/sky.fbx", 1000.0f, true);
}

void Sky_Finalize()
{
    ModelRelease(g_pModelSky);
}

void Sky_SetPosition(const DirectX::XMFLOAT3& position)
{
    g_Position = position;
}

void Sky_Draw()
{
    if (!g_pModelSky) return;

    // --- disable depth write so sky is always behind ---
    Direct3D_SetDepthReadOnly(true);

    // --- shader setup ---
    Shader3dUnlit_Begin();

    // --- world matrix ---
    XMMATRIX world = XMMatrixTranslationFromVector(XMLoadFloat3(&g_Position));
    Shader3dUnlit_SetWorldMatrix(world);
    Shader3dUnlit_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    // --- draw the model ---
    ModelUnlitDraw(g_pModelSky, world);

    // --- restore depth write ---
    Direct3D_SetDepthReadOnly(false);
}
