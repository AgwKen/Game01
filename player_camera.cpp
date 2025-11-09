/*========================================================================================

    Player Camera View [Player_camera.cpp]                      PYAE SONE THANT
                                                                DATE:10/31/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "player_camera.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader3d.h"
#include "player.h"
#include "shader_field.h"
#include "camera.h"
#include "key_logger.h"
#include <algorithm>

static XMFLOAT3 g_PlayerCameraFront = { 0.0f, 0.0f, 1.0f };
static XMFLOAT3 g_PlayerCameraPosition = { 0.0f, 0.0f, 0.0f };

static float g_Yaw = 0.0f;
static float g_Pitch = 0.0f;

static CameraMode g_CameraMode = CameraMode::PLAYER_FOLLOW;

void PlayerCamera_Initialize()
{

}

void PlayerCamera_Finalize()
{
}

// player_camera.cpp (Modified PlayerCamera_Update)

void PlayerCamera_Update(double elapsed_time)
{
    // The static declaration inside the function is unnecessary and prevents the state from persisting.
    // Use the global static variables declared outside the function.
    // static float g_Yaw = 0.0f; // REMOVED (Use global g_Yaw/g_Pitch)
    // static float g_Pitch = 0.0f; // REMOVED

    // Only update the player-following camera logic if we are in that mode
    if (g_CameraMode == CameraMode::PLAYER_FOLLOW)
    {
        // === 1. Handle camera rotation input (can replace with mouse input later) ===
        const float ROT_SPEED = 2.0f * (float)elapsed_time; // rotation speed

        if (KeyLogger_IsPressed(KK_LEFT))  g_Yaw -= ROT_SPEED;
        if (KeyLogger_IsPressed(KK_RIGHT)) g_Yaw += ROT_SPEED;
        if (KeyLogger_IsPressed(KK_UP))    g_Pitch += ROT_SPEED;
        if (KeyLogger_IsPressed(KK_DOWN))  g_Pitch -= ROT_SPEED;

        // Limit pitch (to prevent flipping the camera)
        g_Pitch = std::max(-XM_PIDIV4, std::min(XM_PIDIV4, g_Pitch));

        // === 2. Compute camera forward direction from yaw and pitch ===
        XMVECTOR front = XMVectorSet(
            cosf(g_Pitch) * sinf(g_Yaw),
            sinf(g_Pitch),
            cosf(g_Pitch) * cosf(g_Yaw),
            0.0f
        );
        front = XMVector3Normalize(front);

        XMStoreFloat3(&g_PlayerCameraFront, front);

        // === 3. Compute camera position ===
        XMVECTOR playerPos = XMLoadFloat3(&Player_GetPosition());

        // Camera offset behind and slightly above player
        XMVECTOR offset = -front * 5.0f + XMVectorSet(0.0f, 3.0f, 0.0f, 0.0f);

        XMVECTOR cameraPos = playerPos + offset;
        XMStoreFloat3(&g_PlayerCameraPosition, cameraPos);

        // === 4. Build View matrix (Must be set every frame for the current camera) ===
        XMMATRIX view = XMMatrixLookAtLH(cameraPos, playerPos, XMVectorSet(0, 1, 0, 0));
        Shader3d_SetViewMatrix(view);
        ShaderField_SetViewMatrix(view);

        // === 5. Build Projection matrix (Must be set every frame for the current camera) ===
        float aspectRatio = static_cast<float>(Direct3D_GetBackBufferWidth()) /
            static_cast<float>(Direct3D_GetBackBufferHeight());

        float nearz = 0.1f;
        float farz = 200.0f;
        XMMATRIX projection = XMMatrixPerspectiveFovLH(1.0f, aspectRatio, nearz, farz);

        Shader3d_SetProjectionMatrix(projection);
        ShaderField_SetProjectionMatrix(projection);
    }
}
const DirectX::XMFLOAT3& PlayerCamera_GetFront()
{
    return g_PlayerCameraFront; // Use the actual camera front, not player front
}

const DirectX::XMFLOAT3& PlayerCamera_GetPosition()
{
    return g_PlayerCameraPosition; // Use the actual camera position
}

void PlayerCamera_ToggleMode()
{
    if (g_CameraMode == CameraMode::PLAYER_FOLLOW)
    {
        g_CameraMode = CameraMode::DEBUG_FREE;
    }
    else
    {
        g_CameraMode = CameraMode::PLAYER_FOLLOW;
    }
}

CameraMode PlayerCamera_GetMode()
{
    return g_CameraMode;
}