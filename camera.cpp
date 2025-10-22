/*========================================================================================

    Camera View [camera.h]                                      PYAE SONE THANT
                                                                DATE:09/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "camera.h"
#include "debug_text.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader3d.h"
#include "key_logger.h"
#include <sstream>

static XMFLOAT3 g_CameraPosition = { 0.0f, 0.0f, -5.0f };
static XMFLOAT3 g_CameraFront = { 0.0f, 0.0f,  1.0f };
static XMFLOAT3 g_CameraUp = { 0.0f, 1.0f,  0.0f };
static XMFLOAT3 g_CameraRight = { 1.0f, 0.0f,  0.0f };

static constexpr float CAMERA_MOVE_SPEED = 5.0f;
static constexpr float CAMERA_ROTATION_SPEED = XMConvertToRadians(60); // 30 degrees per second

static XMFLOAT4X4 g_CameraMatrix;
static XMFLOAT4X4 g_PerspectiveMatrix;
static float g_Fov = XMConvertToRadians(60.0f);
static hal::DebugText* g_pDT = nullptr;

void Camera_Initialize(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& front, const DirectX::XMFLOAT3& right)
{
    Camera_Initialize();

    g_CameraPosition = position;
    XMVECTOR f = XMVector3Normalize(XMLoadFloat3(&front));
    XMVECTOR r = XMVector3Normalize(XMLoadFloat3(&right) * XMVECTOR { 1.0f, 0.0f, 1.0f });
    XMVECTOR u = XMVector3Normalize(XMVector3Cross(f, r));
    XMStoreFloat3(&g_CameraFront, f);
    XMStoreFloat3(&g_CameraRight, r);
        XMStoreFloat3(&g_CameraUp, u);
    }

void Camera_Initialize()
{
    g_CameraPosition = { 0.0f, 0.0f, -5.0f };
    g_CameraFront = { 0.0f, 0.0f,  1.0f };
    g_CameraUp = { 0.0f, 1.0f,  0.0f };
    g_CameraRight = { 1.0f, 0.0f,  0.0f };
	g_Fov = XMConvertToRadians(60.0f);

    XMStoreFloat4x4(&g_CameraMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&g_PerspectiveMatrix, XMMatrixIdentity());

#if defined(DEBUG) || defined(_DEBUG)
    g_pDT = new hal::DebugText(Direct3D_GetDevice(), Direct3D_GetDeviceContext(),
        L"Texture/consolab_ascii_512.png",
        Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight(),
        0.0f, 32.0f,
        0, 0,
        0.0f, 14.0f);
#endif

}

void Camera_Finalize()
{
    delete g_pDT;
    g_pDT = nullptr;
}

void Camera_Update(double elapsed_time)
{
    XMVECTOR position = XMLoadFloat3(&g_CameraPosition);
    XMVECTOR front = XMLoadFloat3(&g_CameraFront);
    XMVECTOR up = XMLoadFloat3(&g_CameraUp);
    XMVECTOR right = XMLoadFloat3(&g_CameraRight);

    // rotation (Left/Right)
    if (KeyLogger_IsPressed(KK_RIGHT)) {
        XMMATRIX rotation = XMMatrixRotationY(CAMERA_ROTATION_SPEED * static_cast<float>(elapsed_time));
        up = XMVector3Normalize(XMVector3TransformNormal(up, rotation));
        front = XMVector3TransformNormal(front, rotation);
        front = XMVector3Normalize(front);
        right = XMVector3Normalize(XMVector3Cross(up, front));
    }
    if (KeyLogger_IsPressed(KK_LEFT)) {
        XMMATRIX rotation = XMMatrixRotationY(-CAMERA_ROTATION_SPEED * static_cast<float>(elapsed_time));
        up = XMVector3Normalize(XMVector3TransformNormal(up, rotation));
        front = XMVector3TransformNormal(front, rotation);
        front = XMVector3Normalize(front);
        right = XMVector3Normalize(XMVector3Cross(up, front));
    }

    // rotation (Up/Down)
    if (KeyLogger_IsPressed(KK_UP)) {
        XMMATRIX rotation = XMMatrixRotationAxis(right, -CAMERA_ROTATION_SPEED * static_cast<float>(elapsed_time));
        front = XMVector3TransformNormal(front, rotation);
    }
    if (KeyLogger_IsPressed(KK_DOWN)) {
        XMMATRIX rotation = XMMatrixRotationAxis(right, CAMERA_ROTATION_SPEED * static_cast<float>(elapsed_time));
        front = XMVector3TransformNormal(front, rotation);
    }

    front = XMVector3Normalize(front);

    right = XMVector3Normalize(XMVector3Cross(up, front));
    up = XMVector3Cross(front, right);

    // Movement
    if (KeyLogger_IsPressed(KK_W)) {
        position += XMVector3Normalize(front * XMVECTOR{ 1.0f, 0.0f, 1.0f }) * CAMERA_MOVE_SPEED * static_cast<float>(elapsed_time);
    }
    if (KeyLogger_IsPressed(KK_A)) {
        position -= right * (CAMERA_MOVE_SPEED * static_cast<float>(elapsed_time));
    }
    if (KeyLogger_IsPressed(KK_S)) {
        position += XMVector3Normalize(-front * XMVECTOR{ 1.0f, 0.0f, 1.0f }) * CAMERA_MOVE_SPEED * static_cast<float>(elapsed_time);
    }
    if (KeyLogger_IsPressed(KK_D)) {
        position += right * (CAMERA_MOVE_SPEED * static_cast<float>(elapsed_time));
    }

    // Move up/down (Y axis)
    if (KeyLogger_IsPressed(KK_Q)) {
        position += XMVECTOR{ 0.0f, 1.0f, 0.0f } *CAMERA_MOVE_SPEED * static_cast<float>(elapsed_time);
    }
    if (KeyLogger_IsPressed(KK_E)) {
        position += XMVECTOR{ 0.0f, -1.0f, 0.0f } *CAMERA_MOVE_SPEED * static_cast<float>(elapsed_time);
    }

    if (KeyLogger_IsPressed(KK_Z)) {
		g_Fov -= XMConvertToRadians(10) * static_cast<float>(elapsed_time);
    }
    if (KeyLogger_IsPressed(KK_X)) {
        g_Fov += XMConvertToRadians(10) * static_cast<float>(elapsed_time);
    }

    // Store back to globals
    XMStoreFloat3(&g_CameraPosition, position);
    XMStoreFloat3(&g_CameraFront, front);
    XMStoreFloat3(&g_CameraUp, up);
    XMStoreFloat3(&g_CameraRight, right);

    // View matrix
    XMMATRIX mtxView = XMMatrixLookAtLH(
        position,
        position + front,
        up
    );
    XMStoreFloat4x4(&g_CameraMatrix, mtxView);
    Shader3d_SetViewMatrix(mtxView);

    // Projection matrix
    
    float aspectRatio = static_cast<float>(Direct3D_GetBackBufferWidth()) / static_cast<float>(Direct3D_GetBackBufferHeight());
    float nearz = 0.1f;
    float farz = 200.0f;
    XMMATRIX mtxPerspective = XMMatrixPerspectiveFovLH(g_Fov, aspectRatio, nearz, farz);
    XMStoreFloat4x4(&g_PerspectiveMatrix, mtxPerspective);
    Shader3d_SetProjectionMatrix(mtxPerspective);
}

const DirectX::XMFLOAT4X4& Camera_GetMatrix()
{
    return g_CameraMatrix;
}

const DirectX::XMFLOAT4X4& Camera_GetPerspectiveMatrix()
{
    return g_PerspectiveMatrix;
}

const DirectX::XMFLOAT3& Camera_GetPosition()
{
    return g_CameraPosition;
}

const DirectX::XMFLOAT3& Camera_GetFront()
{
    return g_CameraFront;
}

float Camera_GetFov()
{
    return 0.0f;
}

void Camera_DebugDraw()
{
#if defined(DEBUG) || defined(_DEBUG)
    if (!g_pDT) return;

    std::stringstream ss;

    // Camera position
    ss << "Camera Position: x = " << g_CameraPosition.x;
    ss << " y = " << g_CameraPosition.y;
    ss << " z = " << g_CameraPosition.z << std::endl;

    // Camera front
    ss << "Camera Front: x = " << g_CameraFront.x;
    ss << " y = " << g_CameraFront.y;
    ss << " z = " << g_CameraFront.z << std::endl;

    // Camera right
    ss << "Camera Right: x = " << g_CameraRight.x;
    ss << " y = " << g_CameraRight.y;
    ss << " z = " << g_CameraRight.z << std::endl;

    // Camera up
    ss << "Camera Up: x = " << g_CameraUp.x;
    ss << " z = " << g_CameraUp.z << std::endl;

    ss << "Camera Fov =" << g_Fov << std::endl;
    ss << " y = " << g_CameraUp.y;

    g_pDT->SetText(ss.str().c_str(), { 0.0f, 1.0f, 0.0f, 1.0f });
    g_pDT->Draw();
    g_pDT->Clear();
#endif
}
