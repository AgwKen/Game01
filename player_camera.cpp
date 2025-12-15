/*========================================================================================

    Player Camera View [Player_camera.cpp]                      PYAE SONE THANT
                                                                DATE:10/31/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "player_camera.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "player.h"
#include "camera.h"
#include "key_logger.h"
#include "billboard.h"
#include <algorithm>
#include "direct3d.h"

static XMFLOAT3 g_PlayerCameraFront = { 0.0f, -0.0f, 1.0f };
static XMFLOAT3 g_PlayerCameraPosition = { 0.0f, 0.0f, 0.0f };
static XMFLOAT4X4 g_CameraMatrix{};
static XMFLOAT4X4 g_CameraPerspective{};

static CameraMode g_CameraMode = CameraMode::PLAYER_FOLLOW;

void PlayerCamera_Initialize()
{
}

void PlayerCamera_Finalize()
{
}

void PlayerCamera_Update(double elapsed_time)
{
    if (g_CameraMode == CameraMode::PLAYER_FOLLOW)
    {

        // === Player position ===
        XMVECTOR playerPos = XMLoadFloat3(&Player_GetPosition());

        XMVECTOR offset = XMVectorSet(0.0f, 2.0f, -4.0f, 0.0f);

        // === Camera position ===
        XMVECTOR cameraPos = playerPos + offset;
        XMStoreFloat3(&g_PlayerCameraPosition, cameraPos);

        // === Fixed "look at" direction (at the player) ===
        XMVECTOR lookAtTarget = playerPos;
        XMStoreFloat3(&g_PlayerCameraFront,
            XMVector3Normalize(lookAtTarget - cameraPos));

        // === View matrix ===
        XMMATRIX view = XMMatrixLookAtLH(
            cameraPos,
            lookAtTarget,
            XMVectorSet(0, 1, 0, 0)
        );
        XMStoreFloat4x4(&g_CameraMatrix, view);

        // === Projection ===
        constexpr float fov = XMConvertToRadians(60.0f);
        float aspectRatio =
            (float)Direct3D_GetBackBufferWidth() /
            (float)Direct3D_GetBackBufferHeight();
        float nearz = 0.1f;
        float farz = 200.0f;

        XMMATRIX mtxPerspective =
            XMMatrixPerspectiveFovLH(fov, aspectRatio, nearz, farz);
        XMStoreFloat4x4(&g_CameraPerspective, mtxPerspective);
    }
}

const DirectX::XMFLOAT3& PlayerCamera_GetFront()
{
    return g_PlayerCameraFront;
}

const DirectX::XMFLOAT3& PlayerCamera_GetPosition()
{
    return g_PlayerCameraPosition;
}

void PlayerCamera_ToggleMode()
{
    if (g_CameraMode == CameraMode::PLAYER_FOLLOW)
        g_CameraMode = CameraMode::DEBUG_FREE;
    else
        g_CameraMode = CameraMode::PLAYER_FOLLOW;
}

CameraMode PlayerCamera_GetMode()
{
    return g_CameraMode;
}

const XMFLOAT4X4& PlayerCamera_GetViewMatrix()
{
    return g_CameraMatrix;
}

const DirectX::XMFLOAT4X4& PlayerCamera_GetPerspectiveMatrix()
{
    return g_CameraPerspective;
}
