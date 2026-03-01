#include "Goal.h"
#include "GoalCollision.h"
#include "terrain.h"
#include "cube.h"
#include "direct3d.h"
#include "shader3d.h"
#include <vector>

using namespace DirectX;

static MODEL* g_GoalModel = nullptr;

static XMFLOAT3 g_GoalModelPosition = { 7.0f, 0.0f, 2.5f };

static float g_GoalModelScale = 1.2f;
static float g_ModelRotationY = -XM_PIDIV2;

static float g_ModelHeightOffset = 1.0f;

static bool g_ShowPoleDebug = true;
static bool g_ShowNetDebug = true;

static XMMATRIX g_GoalWorld;

extern XMFLOAT3 g_GoalWorldOffset;

static XMFLOAT3 g_GoalWorldPos = { 7.0f, 0.0f, 2.5f };

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
        g_GoalWorldPos.x,
        g_GoalWorldPos.z
    );

    float finalY = terrainY + (g_ModelHeightOffset * g_GoalModelScale);
    g_GoalWorldBaseY = finalY - (g_ModelHeightOffset * g_GoalModelScale);

    g_GoalWorld =
        XMMatrixScaling(g_GoalModelScale, g_GoalModelScale, g_GoalModelScale) *
        XMMatrixRotationY(g_ModelRotationY) *
        XMMatrixTranslation(
            g_GoalWorldPos.x,
            finalY,
            g_GoalWorldPos.z
        );

    ModelDraw(g_GoalModel, g_GoalWorld);
 // Always build collision boxes
std::vector<AABB> boxes;
GoalCollision_GetDebugBoxes(boxes);

// ------------------------------------------------------------
// POLE DEBUG BOXES (ALWAYS DRAW)
// ------------------------------------------------------------
if (g_ShowPoleDebug)
{
    for (int i = 0; i < 3 && i < (int)boxes.size(); i++)
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

#if defined(DEBUG) || defined(_DEBUG)

// ------------------------------------------------------------
// GOAL LINE DEBUG (DEBUG ONLY)
// ------------------------------------------------------------
if (boxes.size() >= 3)
{
    const AABB& left = boxes[0];
    const AABB& right = boxes[1];

    float goalLineZ = (left.min.z + left.max.z) * 0.5f;

    float minX = left.max.x;
    float maxX = right.min.x;

    float baseY = left.min.y + 0.05f;

    float width = (maxX - minX);
    float thickness = 0.05f;
    float depth = 0.05f;

    XMMATRIX debugGoalLine =
        XMMatrixScaling(width, thickness, depth) *
        XMMatrixTranslation(
            (minX + maxX) * 0.5f,
            baseY,
            goalLineZ
        );

    Direct3D_SetAlphaBlendState();
    CUBE_Draw(-1, debugGoalLine);
    Direct3D_SetDefaultBlendState();
}

// ------------------------------------------------------------
// NET DEBUG BOXES (DEBUG ONLY)
// ------------------------------------------------------------
if (g_ShowNetDebug)
{
    for (int i = 3; i < (int)boxes.size(); i++)
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
bool Goal_CheckScored(const XMFLOAT3& pos, float r)
{
    return GoalCollision_IsBallInsideGoal(pos, r);
}

XMMATRIX Goal_GetWorldMatrix()
{
    return g_GoalWorld;
}

MODEL* Goal_GetModel()
{
    return g_GoalModel;
}

void Goal_SetWorldOffset(const XMFLOAT3& offset)
{
    g_GoalWorldPos.x = g_GoalModelPosition.x + offset.x;
    g_GoalWorldPos.y = g_GoalModelPosition.y + offset.y;
    g_GoalWorldPos.z = g_GoalModelPosition.z + offset.z;
}

XMFLOAT3 Goal_GetWorldPosition()
{
    return g_GoalWorldPos;
}