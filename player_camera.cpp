/*========================================================================================

    Player Camera View [Player_camera.cpp]                      PYAE SONE THANT
                                                                DATE:10/31/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "player_camera.h"
#include <DirectXMath.h>
using namespace DirectX;

#include "player.h"
#include "key_logger.h"
#include "pad_logger.h"
#include "direct3d.h"

// Camera State
static XMFLOAT3 g_PlayerCameraPosition = { 0.0f, 0.0f, 0.0f };
static XMFLOAT3 g_PlayerCameraFront = { 0.0f, 0.0f, 1.0f };
static XMFLOAT4X4 g_CameraMatrix{};
static XMFLOAT4X4 g_CameraPerspective{};

static CameraMode g_CameraMode = CameraMode::PLAYER_FOLLOW;

// Peek System State
static XMFLOAT2 g_PeekOffset = { 0.0f, 0.0f };
static XMFLOAT2 g_TargetPeekOffset = { 0.0f, 0.0f };

// Tunable Parameters
static constexpr float CAMERA_HEIGHT = 1.8f;
static constexpr float CAMERA_DISTANCE = -4.0f;

static constexpr float PEEK_DISTANCE = 2.0f;
static constexpr float PEEK_SPEED = 3.0f;
static constexpr float PEEK_RETURN_SPEED = 3.0f;
static constexpr float PEEK_DEADZONE = 0.2f;

static constexpr float PEEK_Y_SCALE = 1.0f;   //  no more than 1
static constexpr float PEEK_DOWN_SCALE = 0.6f; // optional

void PlayerCamera_Initialize()
{
}

void PlayerCamera_Finalize()
{
}

void PlayerCamera_Update(double elapsed_time)
{
    if (g_CameraMode != CameraMode::PLAYER_FOLLOW)
        return;

    const float dt = static_cast<float>(elapsed_time);

    float inputX = 0.0f;
    float inputY = 0.0f;

    if (KeyLogger_IsPressed(KK_LEFT))  inputX -= 1.0f;
    if (KeyLogger_IsPressed(KK_RIGHT)) inputX += 1.0f;
    if (KeyLogger_IsPressed(KK_UP))    inputY += 1.0f;
    if (KeyLogger_IsPressed(KK_DOWN))  inputY -= 1.0f;

    //controller
    XMFLOAT2 stick = PadLogger_GetLeftThumbStick(0);
    inputX += stick.x;
    inputY += stick.y;


    XMVECTOR inputVec = XMVectorSet(inputX, inputY, 0.0f, 0.0f);
    float inputLen = XMVectorGetX(XMVector2Length(inputVec));

    if (inputLen > PEEK_DEADZONE)
    {
        inputVec = XMVector2Normalize(inputVec);
        inputVec *= PEEK_DISTANCE;
        XMStoreFloat2(&g_TargetPeekOffset, inputVec);
    }
    else
    {
        g_TargetPeekOffset = { 0.0f, 0.0f };

    }
    //SMOOTH INTERPOLATION (PEEK / RETURN)
    float speed = (g_TargetPeekOffset.x != 0.0f || g_TargetPeekOffset.y != 0.0f)
        ? PEEK_SPEED
        : PEEK_RETURN_SPEED;

    g_PeekOffset.x += (g_TargetPeekOffset.x - g_PeekOffset.x) * speed * dt;
    g_PeekOffset.y += (g_TargetPeekOffset.y - g_PeekOffset.y) * speed * dt;

    if (g_PeekOffset.y > 0.0f)       // looking UP
        g_PeekOffset.y *= PEEK_Y_SCALE;
    else                             // looking DOWN
        g_PeekOffset.y *= PEEK_DOWN_SCALE;


    //CAMERA POSITION (FOLLOW PLAYER
    XMVECTOR playerPos = XMLoadFloat3(&Player_GetPosition());

    XMVECTOR cameraOffset = XMVectorSet(
        0.0f,
        CAMERA_HEIGHT,
        CAMERA_DISTANCE,
        0.0f
    );

    XMVECTOR cameraPos = playerPos + cameraOffset;
    XMStoreFloat3(&g_PlayerCameraPosition, cameraPos);


    //TARGET (PLAYER + PEEK OFFSET
    XMVECTOR lookTarget = playerPos +
        XMVectorSet(
            g_PeekOffset.x,
            g_PeekOffset.y,
            0.0f,
            0.0f
        );

    XMVECTOR front = XMVector3Normalize(lookTarget - cameraPos);
    XMStoreFloat3(&g_PlayerCameraFront, front);


    XMMATRIX view = XMMatrixLookAtLH(
        cameraPos,
        lookTarget,
        XMVectorSet(0, 1, 0, 0)
    );
    XMStoreFloat4x4(&g_CameraMatrix, view);

    // PROJECTION 
    constexpr float fov = XMConvertToRadians(60.0f);
    float aspect =
        static_cast<float>(Direct3D_GetBackBufferWidth()) /
        static_cast<float>(Direct3D_GetBackBufferHeight());

    XMMATRIX proj =
        XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 200.0f);

    XMStoreFloat4x4(&g_CameraPerspective, proj);
}

const XMFLOAT3& PlayerCamera_GetPosition()
{
    return g_PlayerCameraPosition;
}

const XMFLOAT3& PlayerCamera_GetFront()
{
    return g_PlayerCameraFront;
}

const XMFLOAT4X4& PlayerCamera_GetViewMatrix()
{
    return g_CameraMatrix;
}

const XMFLOAT4X4& PlayerCamera_GetPerspectiveMatrix()
{
    return g_CameraPerspective;
}

CameraMode PlayerCamera_GetMode()
{
    return g_CameraMode;
}

void PlayerCamera_ToggleMode()
{
    if (g_CameraMode == CameraMode::PLAYER_FOLLOW)
        g_CameraMode = CameraMode::DEBUG_FREE;
    else
        g_CameraMode = CameraMode::PLAYER_FOLLOW;
}
