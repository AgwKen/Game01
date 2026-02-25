/*========================================================================================

    Player Camera View [Player_camera.cpp]                      PYAE SONE THANT
                                                                DATE:10/31/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#include "player_camera.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "key_logger.h"
#include "pad_logger.h"
#include "direct3d.h"
#include <algorithm>
#include "BallPlayer.h"

// ------------------------------------------------------------
// MANUAL CLAMP (no std::clamp)
// ------------------------------------------------------------
static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

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
static constexpr float CAMERA_HEIGHT = 1.2f;

// Normal camera distance (your original)
static constexpr float CAMERA_DISTANCE_NORMAL = -4.0f;

// Zoom OUT distance while charging (more negative = farther back)
static constexpr float CAMERA_DISTANCE_CHARGE = -7.5f;

// Close-up distance
static constexpr float CAMERA_DISTANCE_IMPACT = 1.0f;

// Current smoothed camera distance
static float g_CameraDistanceCurrent = CAMERA_DISTANCE_NORMAL;

// Speeds
static constexpr float CAMERA_ZOOM_SPEED = 4.0f;   // while holding charge
static constexpr float CAMERA_SNAP_SPEED = 200.0f;  // snap back
static constexpr float PEEK_DISTANCE = 2.0f;
static constexpr float PEEK_SPEED = 3.0f;
static constexpr float PEEK_RETURN_SPEED = 0.1f;
static constexpr float PEEK_DEADZONE = 0.2f;

static constexpr float PEEK_Y_SCALE = 1.0f;   //  no more than 1
static constexpr float PEEK_DOWN_SCALE = 0.6f; // optional

static constexpr float CAMERA_FOLLOW_SPEED = 3.0f; // smaller = slower, heavier camera

// ------------------------------------------------------------
// NEW: Kick cinematic lock
// ------------------------------------------------------------
static bool g_KickCinematicActive = false;

// API for BallPlayer
void PlayerCamera_BeginKickCinematic()
{
    g_KickCinematicActive = true;
}
void PlayerCamera_EndKickCinematic()
{
    g_KickCinematicActive = false;
}
void PlayerCamera_SnapCloseNow()
{
    g_CameraDistanceCurrent = CAMERA_DISTANCE_IMPACT;
}
void PlayerCamera_ResetKickCinematic()
{
    g_KickCinematicActive = false;
    g_CameraDistanceCurrent = CAMERA_DISTANCE_NORMAL;
}

void PlayerCamera_Initialize()
{
    PlayerCamera_ResetKickCinematic();

    XMFLOAT3 playerPosFloat = BallPlayer_GetPosition();
    XMVECTOR playerPos = XMLoadFloat3(&playerPosFloat);

    g_CameraDistanceCurrent = CAMERA_DISTANCE_NORMAL;

    XMVECTOR offset = XMVectorSet(0.0f, CAMERA_HEIGHT, g_CameraDistanceCurrent, 0.0f);
    XMVECTOR startPos = playerPos + offset;

    XMStoreFloat3(&g_PlayerCameraPosition, startPos);
}

void PlayerCamera_Finalize()
{
}

void PlayerCamera_Update(double elapsed_time)
{
    if (g_CameraMode != CameraMode::PLAYER_FOLLOW)
        return;

    const float dt = static_cast<float>(elapsed_time);

    // ------------------------------------------------------------
    // INPUT (PEEK)
    // ------------------------------------------------------------
    float inputX = 0.0f;
    float inputY = 0.0f;

    if (KeyLogger_IsPressed(KK_LEFT))  inputX -= 1.0f;
    if (KeyLogger_IsPressed(KK_RIGHT)) inputX += 1.0f;
    if (KeyLogger_IsPressed(KK_UP))    inputY += 1.0f;
    if (KeyLogger_IsPressed(KK_DOWN))  inputY -= 1.0f;

    XMFLOAT2 stick = PadLogger_GetRightThumbStick(0);
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

    float speed = (g_TargetPeekOffset.x != 0.0f || g_TargetPeekOffset.y != 0.0f)
        ? PEEK_SPEED
        : PEEK_RETURN_SPEED;

    g_PeekOffset.x += (g_TargetPeekOffset.x - g_PeekOffset.x) * speed * dt;
    g_PeekOffset.y += (g_TargetPeekOffset.y - g_PeekOffset.y) * speed * dt;

    if (g_PeekOffset.y > 0.0f)
        g_PeekOffset.y *= PEEK_Y_SCALE;
    else
        g_PeekOffset.y *= PEEK_DOWN_SCALE;

    // ------------------------------------------------------------
    // CAMERA FOLLOW
    // ------------------------------------------------------------
    XMFLOAT3 playerPosFloat = BallPlayer_GetPosition();
    XMVECTOR playerPos = XMLoadFloat3(&playerPosFloat);

    float targetDistance = CAMERA_DISTANCE_NORMAL;

    // ------------------------------------------------------------
    // 1) If kick cinematic active -> FORCE close-up
    // ------------------------------------------------------------
    if (g_KickCinematicActive)
    {
        targetDistance = CAMERA_DISTANCE_IMPACT;
        g_CameraDistanceCurrent = targetDistance; // hard lock
    }
    else
    {
        // --------------------------------------------------------
        // 2) Normal charge zoom out
        // --------------------------------------------------------
        float charge01 = 0.0f;

        if (BallPlayer_IsCharging())
        {
            float denom = BallPlayer_GetKickMaxPower();
            if (denom <= 0.0001f) denom = 1.0f;
            charge01 = BallPlayer_GetKickCharge() / denom;
            charge01 = Clamp01(charge01);
        }

        targetDistance =
            CAMERA_DISTANCE_NORMAL +
            (CAMERA_DISTANCE_CHARGE - CAMERA_DISTANCE_NORMAL) * charge01;

        float distSpeed = BallPlayer_IsCharging() ? CAMERA_ZOOM_SPEED : CAMERA_SNAP_SPEED;

        float k = distSpeed * dt;
        if (k > 1.0f) k = 1.0f;

        g_CameraDistanceCurrent += (targetDistance - g_CameraDistanceCurrent) * k;
    }

    XMVECTOR cameraOffset = XMVectorSet(
        0.0f,
        CAMERA_HEIGHT,
        g_CameraDistanceCurrent,
        0.0f
    );

    XMVECTOR desiredCameraPos = playerPos + cameraOffset;
    XMVECTOR currentCameraPos = XMLoadFloat3(&g_PlayerCameraPosition);

    float t = CAMERA_FOLLOW_SPEED * dt;
    t = std::min(t, 1.0f);

    XMFLOAT3 cur, des;
    XMStoreFloat3(&cur, currentCameraPos);
    XMStoreFloat3(&des, desiredCameraPos);

    cur.x += (des.x - cur.x) * t;
    cur.z += (des.z - cur.z) * t;
    cur.y = des.y;

    currentCameraPos = XMLoadFloat3(&cur);
    XMStoreFloat3(&g_PlayerCameraPosition, currentCameraPos);

    XMVECTOR cameraPos = currentCameraPos;

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