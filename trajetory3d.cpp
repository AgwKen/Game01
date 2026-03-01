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
    XMFLOAT3 position{};
    XMFLOAT4 color{ 1,1,1,1 };
    float    size = 1.0f;
    double   lifeTime = 0.2;
    double   birthTime = -1.0; // IMPORTANT: -1 means inactive (NOT 0.0)
};

static constexpr unsigned int TRAJECTORY_MAX = 4096;
static Trajectory3d g_Trajectorys[TRAJECTORY_MAX]{};

static int    g_TrajectoryTexId = -1;
static double g_Time = 0.0;

void Trajetory3d_Initialize()
{
    g_TrajectoryTexId = Texture_Load(L"Texture/effect000.jpg");
    g_Time = 0.0;

    for (auto& t : g_Trajectorys)
        t.birthTime = -1.0; // inactive
}

void Trajetory3d_Finalize()
{
    if (g_TrajectoryTexId >= 0)
    {
        Texture_Release(g_TrajectoryTexId);
        g_TrajectoryTexId = -1;
    }

    for (auto& t : g_Trajectorys)
        t.birthTime = -1.0;

    g_Time = 0.0;
}

void Trajetory3d_Update(double elapsed_time)
{
    g_Time += elapsed_time;

    for (auto& t : g_Trajectorys)
    {
        if (t.birthTime < 0.0) continue;

        double time = g_Time - t.birthTime;
        if (time > t.lifeTime)
        {
            t.birthTime = -1.0; // kill
        }
    }
}

void Trajetory3d_Draw()
{
    if (g_TrajectoryTexId < 0) return;

    Direct3D_SetAdditiveBlendState();

    Direct3D_SetDepthReadOnly(true);

    for (const auto& t : g_Trajectorys)
    {
        if (t.birthTime < 0.0) continue;

        double time = g_Time - t.birthTime;
        float ratio = (float)(time / t.lifeTime);

        // shrink over time
        float size = t.size * (1.0f - ratio);

        // fade alpha
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

    Direct3D_SetDepthReadOnly(false);
    Direct3D_SetDefaultBlendState();
}

void Trajectory3d_Create(const XMFLOAT3& position, const XMFLOAT4& color, float size, double lifeTime)
{
    for (auto& t : g_Trajectorys)
    {
        if (t.birthTime >= 0.0) continue; // occupied

        t.position = position;
        t.color = color;
        t.size = size;
        t.lifeTime = lifeTime;
        t.birthTime = g_Time;
        break;
    }
}