#include "Goal.h"
#include "GoalCollision.h"

#include "model.h"
#include "terrain.h"
#include "cube.h"
#include "direct3d.h"

#include <vector>

using namespace DirectX;

static MODEL* g_GoalModel = nullptr;

static XMFLOAT3 g_GoalModelPosition = { 7.0f, 0.0f, 2.5f };

static float g_GoalModelScale = 1.2f;
static float g_ModelRotationY = -XM_PIDIV2;

static float g_ModelHeightOffset = 1.0f;

static bool g_ShowPoleDebug = true;
static bool g_ShowNetDebug = true;

extern XMFLOAT3 g_GoalWorldOffset;


void Goal_Initialize()
{
    g_GoalModel = ModelLoad("Resources/Model/goal.fbx", 1.0f);

    GoalCollision_Initialize();
}

void Goal_Finalize()
{
    if (g_GoalModel)
    {
        ModelRelease(g_GoalModel);
        g_GoalModel = nullptr;
    }

    GoalCollision_Finalize();
}

bool Goal_HandleBallCollision(
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius)
{
    return GoalCollision_HandleBall(ballPos, ballVelocity, ballRadius);
}

void Goal_Render()
{
    if (!g_GoalModel)
        return;

    float terrainY = Mesh_GetHeightAt(
        g_GoalModelPosition.x + g_GoalWorldOffset.x,
        g_GoalModelPosition.z + g_GoalWorldOffset.z
    );

    float finalY = terrainY + (g_ModelHeightOffset * g_GoalModelScale);
    g_GoalWorldBaseY = finalY - (g_ModelHeightOffset * g_GoalModelScale);

    XMMATRIX world =
        XMMatrixScaling(g_GoalModelScale, g_GoalModelScale, g_GoalModelScale) *
        XMMatrixRotationY(g_ModelRotationY) *
        XMMatrixTranslation(
            g_GoalModelPosition.x + g_GoalWorldOffset.x,
            finalY,
            g_GoalModelPosition.z + g_GoalWorldOffset.z
        );


    ModelDraw(g_GoalModel, world);

#if defined(DEBUG) || defined(_DEBUG)

    std::vector<AABB> boxes;
    GoalCollision_GetDebugBoxes(boxes);

    if (g_ShowPoleDebug)
    {
        for (int i = 0; i < 3; i++)
        {
            AABB& box = boxes[i];

            XMFLOAT3 center = {
                (box.min.x + box.max.x) * 0.5f,
                (box.min.y + box.max.y) * 0.5f,
                (box.min.z + box.max.z) * 0.5f
            };

            XMFLOAT3 size = {
                box.max.x - box.min.x,
                box.max.y - box.min.y,
                box.max.z - box.min.z
            };

            XMMATRIX debugWorld =
                XMMatrixScaling(size.x, size.y, size.z) *
                XMMatrixTranslation(center.x, center.y, center.z);

            Direct3D_SetAlphaBlendState();
            CUBE_Draw(-1, debugWorld);
            Direct3D_SetDefaultBlendState();
        }
    }

    if (g_ShowNetDebug)
    {
        for (int i = 3; i < boxes.size(); i++)
        {
            AABB& box = boxes[i];

            XMFLOAT3 center = {
                (box.min.x + box.max.x) * 0.5f,
                (box.min.y + box.max.y) * 0.5f,
                (box.min.z + box.max.z) * 0.5f
            };

            XMFLOAT3 size = {
                box.max.x - box.min.x,
                box.max.y - box.min.y,
                box.max.z - box.min.z
            };

            XMMATRIX debugWorld =
                XMMatrixScaling(size.x, size.y, size.z) *
                XMMatrixTranslation(center.x, center.y, center.z);

            Direct3D_SetAlphaBlendState();
            CUBE_Draw(-1, debugWorld);
            Direct3D_SetDefaultBlendState();
        }
    }

#endif
}

bool Goal_CheckScored(const XMFLOAT3&, float)
{
    return false;
}
