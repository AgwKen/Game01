#include "Goal.h"
#include "GoalCollision.h"
#include "terrain.h"
#include "cube.h"
#include "direct3d.h"
#include "shader3d.h"
#include <vector>
#include <cstdlib>
#include <cmath>

using namespace DirectX;

static MODEL* g_GoalModel = nullptr;

// Base (default) position
static XMFLOAT3 g_GoalModelPosition = { 0.0f, 0.0f, 18.0f };
static XMFLOAT3 g_GoalWorldPos = { 0.0f, 0.0f, 18.0f };

static float g_GoalModelScale = 1.2f;
static float g_ModelRotationY = -XM_PIDIV2;

static float g_ModelHeightOffset = 1.0f;

static bool g_ShowPoleDebug = true;
static bool g_ShowNetDebug = true;

static XMMATRIX g_GoalWorld;

extern XMFLOAT3 g_GoalWorldOffset;

// from GoalCollision.cpp
extern float g_GoalWorldBaseY;

// ? forward declare (IMPORTANT)
static void Goal_UpdateBaseY();

// ------------------------------------------------------------
// Random helpers
// ------------------------------------------------------------
static float Rand01()
{
    return rand() / (float)RAND_MAX;
}
static float RandRange(float a, float b)
{
    return a + (b - a) * Rand01();
}

// ------------------------------------------------------------
// Flatness check (avoid placing goal on extreme slopes)
// ------------------------------------------------------------
static bool IsAreaReasonablyFlat(float x, float z)
{
    // sample around goal footprint
    const float s = 1.5f; // sample distance
    float h0 = Mesh_GetHeightAt(x, z);
    float hx1 = Mesh_GetHeightAt(x + s, z);
    float hx2 = Mesh_GetHeightAt(x - s, z);
    float hz1 = Mesh_GetHeightAt(x, z + s);
    float hz2 = Mesh_GetHeightAt(x, z - s);

    float maxH = h0;
    float minH = h0;

    auto upd = [&](float h)
        {
            if (h > maxH) maxH = h;
            if (h < minH) minH = h;
        };

    upd(hx1); upd(hx2); upd(hz1); upd(hz2);

    // tweak this tolerance based on your terrain (0.6 ~ 1.5)
    return (maxH - minH) < 1.0f;
}

// ------------------------------------------------------------
// NEW: Set exact world position (XZ)
// ------------------------------------------------------------
void Goal_SetWorldPosition(float x, float z)
{
    g_GoalWorldPos.x = x;
    g_GoalWorldPos.z = z;
    Goal_UpdateBaseY();
}

// ------------------------------------------------------------
// NEW: Randomize goal placement
// Terrain is 0..100 (your mesh grid).
// We keep margins so it doesn't clip out of field.
// Also ensure Z is far enough for your AirCurve forward distance.
// ------------------------------------------------------------
void Goal_RandomizePlacement()
{
    // Center around the player's shot area / reset area
    const XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };

    // Pick a random distance band:
    // sometimes close, sometimes medium, sometimes far
    float minDist = 0.0f;
    float maxDist = 0.0f;

    int roll = rand() % 100;
    if (roll < 35)
    {
        // close
        minDist = 10.0f;
        maxDist = 26.0f;
    }
    else if (roll < 75)
    {
        // medium
        minDist = 16.0f;
        maxDist = 30.0f;
    }
    else
    {
        // far
        minDist = 24.0f;
        maxDist = 50.0f;
    }

    const float MIN_X = -35.0f;
    const float MAX_X = 35.0f;
    const float MIN_Z = 12.0f;
    const float MAX_Z = 55.0f;

    for (int tries = 0; tries < 40; ++tries)
    {
        // random left/right
        float side = (Rand01() < 0.5f) ? -1.0f : 1.0f;

        // random sideways amount
        float x = center.x + side * RandRange(4.0f, 18.0f);

        // random forward distance
        float z = center.z + RandRange(minDist, maxDist);

        x = std::max(MIN_X, std::min(x, MAX_X));
        z = std::max(MIN_Z, std::min(z, MAX_Z));

        if (IsAreaReasonablyFlat(x, z))
        {
            g_GoalWorldPos.x = x;
            g_GoalWorldPos.z = z;
            Goal_UpdateBaseY();
            return;
        }
    }

    // fallback
    g_GoalWorldPos.x = RandRange(-12.0f, 12.0f);
    g_GoalWorldPos.z = RandRange(14.0f, 28.0f);
    Goal_UpdateBaseY();
}
void Goal_Initialize()
{
    g_GoalModel = ModelLoad("Resources/Model/goal.fbx", 1.0f);

    // OPTIONAL: randomize once at start
    // Goal_RandomizePlacement();

    GoalCollision_Initialize();

    // ensure baseY valid immediately
    Goal_UpdateBaseY();
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
    const XMFLOAT3& prevBallPos,
    XMFLOAT3& ballPos,
    XMFLOAT3& ballVelocity,
    float ballRadius)
{
    return GoalCollision_HandleBall(prevBallPos, ballPos, ballVelocity, ballRadius);
}

void Goal_Render()
{
    if (!g_GoalModel)
        return;

    float terrainY = Mesh_GetHeightAt(g_GoalWorldPos.x, g_GoalWorldPos.z);

    float finalY = terrainY + (g_ModelHeightOffset * g_GoalModelScale);

    // baseY for collision build
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

    // POLE DEBUG BOXES
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

    // GOAL LINE DEBUG
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

    // NET DEBUG BOXES
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

    Goal_UpdateBaseY();
}

XMFLOAT3 Goal_GetWorldPosition()
{
    return g_GoalWorldPos;
}

// ------------------------------------------------------------
// INTERNAL: Update baseY immediately (for collisions / mouth target)
// ------------------------------------------------------------
static void Goal_UpdateBaseY()
{
    float terrainY = Mesh_GetHeightAt(g_GoalWorldPos.x, g_GoalWorldPos.z);
    float finalY = terrainY + (g_ModelHeightOffset * g_GoalModelScale);
    g_GoalWorldBaseY = finalY - (g_ModelHeightOffset * g_GoalModelScale);
}