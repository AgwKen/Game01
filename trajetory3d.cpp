/*========================================================================================

   trajetory 3D Cpp[trajetory3d.cpp]                                   PYAE SONE THANT
                                                                        DATE:21/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "trajetory3d.h"
#include "texture.h"
#include "billboard.h"
#include "direct3d.h"
#include "shader_billboard.h"
using namespace DirectX;

struct Trajectory3d
{
    XMFLOAT3 position;
    XMFLOAT4 color;
    float size;
    double lifeTime;
    double birthTime;
};

static constexpr unsigned int TRAJECTORY_MAX = 4096;
static Trajectory3d g_Trajectorys[TRAJECTORY_MAX]{};
static int g_TrajectoryTexId = -1;
static double g_Time = 0.0;

void Trajetory3d_Initialize()
{
    g_TrajectoryTexId = Texture_Load(L"Texture/effect000.jpg");
}

void Trajetory3d_Finalize()
{
    if (g_TrajectoryTexId >= 0)
        Texture_Release(g_TrajectoryTexId);
}

void Trajetory3d_Update(double elapsed_time)
{
    g_Time += elapsed_time;

    for (Trajectory3d& t : g_Trajectorys) {
        if (t.birthTime == 0.0) continue;
        double time = g_Time - t.birthTime;
        if (time > t.lifeTime) {
            t.birthTime = 0.0;
        }
    }
}

void Trajetory3d_Draw()
{
    Direct3D_SetAdditiveBlendState();

      for (const Trajectory3d& t : g_Trajectorys) {
        if (t.birthTime == 0.0) continue;

        double time = g_Time - t.birthTime;
        float ratio = (float)(time / t.lifeTime);
        float size = t.size * (1.0f - ratio);

        XMFLOAT4 color = t.color;
        color.w = t.color.w * (1.0f - ratio);

         ShaderBillBoard_SetColor(color);

        Billboard_Draw(
            g_TrajectoryTexId,
            t.position,
            XMFLOAT2(size, size),
            XMFLOAT4(0, 0, 0, 0),
            XMFLOAT2(0.5f, 0.5f),
            color
        );

    }

    Direct3D_SetDefaultBlendState();
}


void Trajectory3d_Create(const XMFLOAT3& position, const XMFLOAT4& color, float size, double lifeTime)
{
    for (Trajectory3d& t : g_Trajectorys) {
        if (t.birthTime != 0.0) continue;

        t.position = position;
        t.color = color;
        t.size = size;
        t.lifeTime = lifeTime;
        t.birthTime = g_Time;
        break;
    }
}
