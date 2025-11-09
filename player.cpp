/*===============================================================================

  player cpp[player.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/10/31
---------------------------------------------------------------------------------

=================================================================================*/

#include "player.h"
#include <DirectXMath.h>
#include "model.h"
#include "key_logger.h"
#include "camera.h"
#include "player_camera.h"
#include "light.h"
#include "cube.h"

using namespace DirectX;

static XMFLOAT3 g_PlayerPosition{ 0.0f, 0.0f, 0.0f };
static XMFLOAT3 g_PlayerFront{ 0.0f, 0.0f, 0.0f };
static XMFLOAT3 g_PlayerVelocity{};
static MODEL* g_pPlayerModel{ nullptr };
static bool g_IsJump = false;
const float PLAYER_SPEED = 5.0f;


void Player_Initialize(const XMFLOAT3& position, const XMFLOAT3& front)
{
    g_PlayerPosition = position;
    g_PlayerVelocity = { 0.0f, 0.0f, 0.0f };
    XMStoreFloat3(&g_PlayerFront, XMVector3Normalize(XMLoadFloat3(&front)));
    g_IsJump = false;
    g_pPlayerModel = ModelLoad("Resources/Model/test.fbx", 0.1f);
}

void Player_Finalize()
{
    ModelRelease(g_pPlayerModel);
}
void Player_Update(double elapsed_time)
{
    // 1. Get old state
    XMVECTOR old_position = XMLoadFloat3(&g_PlayerPosition);
    XMVECTOR velocity = XMLoadFloat3(&g_PlayerVelocity);

    AABB old_player_aabb = Player_GetAABB();
    XMVECTOR position = old_position;

    // 2. Jump Input
    if (KeyLogger_IsTrigger(KK_SPACE) && !g_IsJump)
    {
        velocity += { 0.0f, 40.0f, 0.0f };
        g_IsJump = true;
    }

    // 3. Gravity (Always applied)
    XMVECTOR gdir{ 0.0f,1.0f, 0.0f };
    velocity += gdir * -9.8f * 15.0f * (float)elapsed_time;

    // 3. Movement Input
    XMVECTOR direction{};
    XMVECTOR front = XMLoadFloat3(&PlayerCamera_GetFront()) * XMVECTOR { 1.0f, 0.0f, 1.0f };

    if (KeyLogger_IsPressed(KK_W)) {
        direction += front;
    }
    if (KeyLogger_IsPressed(KK_A)) {
        direction -= XMVector3Cross({ 0.0f,1.0f,0.0f }, front);
    }
    if (KeyLogger_IsPressed(KK_S)) {
        direction -= front;
    }
    if (KeyLogger_IsPressed(KK_D)) {
        direction += XMVector3Cross({ 0.0f,1.0f,0.0f }, front);
    }

    //4. Movement Logic (Only if moving)
    if (XMVectorGetX(XMVector3LengthSq(direction)) > 0.0f) {
        direction = XMVector3Normalize(direction);

        // Rotation Logic
        float dot = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&g_PlayerFront), direction));
        float angle = acosf(dot);
        const float rotation_speed = XM_2PI * 1.0f * (float)elapsed_time;

        if (angle < rotation_speed) {
            front = direction;
        }
        else {

            XMMATRIX r = XMMatrixIdentity();

            if (XMVectorGetY(XMVector3Cross(XMLoadFloat3(&g_PlayerFront), direction)) < 0.0f) {
                r = XMMatrixRotationY(-XM_2PI * 2.0f * (float)elapsed_time);
            }
            else {
                r = XMMatrixRotationY(XM_2PI * 2.0f * (float)elapsed_time);
            }

            front = XMVector3TransformNormal(XMLoadFloat3(&g_PlayerFront), r);
        }// End Rotation

        velocity += front * (float)(2000.0 / 50.0 * elapsed_time); // Apply Movement
        XMStoreFloat3(&g_PlayerFront, front); // Save new direction
    }
    // 5. FINAL PHYSICS (Always applied)
    velocity += -velocity * (float)(4.0f * elapsed_time); // Apply friction
    position += velocity * (float)elapsed_time; // Apply final velocity

    // ===== CUBE COLLISION (stable + correct direction) =====
    AABB cube = Cube_GetAABB({ 3.0f, 0.5f, 2.0f });
    XMStoreFloat3(&g_PlayerPosition, position);
    AABB player = Player_GetAABB();

    Hit hit = Collision_IsHitAABB(cube, player);
    if (hit.isHit)
    {
        const float skin = 0.002f;

        // Calculate overlap amount on each side
        float overlapX1 = cube.max.x - player.min.x;
        float overlapX2 = player.max.x - cube.min.x;
        float overlapY1 = cube.max.y - player.min.y;
        float overlapY2 = player.max.y - cube.min.y;
        float overlapZ1 = cube.max.z - player.min.z;
        float overlapZ2 = player.max.z - cube.min.z;

        // Smallest overlaps
        float minX = (overlapX1 < overlapX2) ? overlapX1 : overlapX2;
        float minY = (overlapY1 < overlapY2) ? overlapY1 : overlapY2;
        float minZ = (overlapZ1 < overlapZ2) ? overlapZ1 : overlapZ2;

        // Determine cube center and player center to know direction
        XMFLOAT3 cubeCenter{
            (cube.min.x + cube.max.x) * 0.5f,
            (cube.min.y + cube.max.y) * 0.5f,
            (cube.min.z + cube.max.z) * 0.5f
        };
        XMFLOAT3 playerCenter{
            (player.min.x + player.max.x) * 0.5f,
            (player.min.y + player.max.y) * 0.5f,
            (player.min.z + player.max.z) * 0.5f
        };

        // Resolve on smallest axis
        if (minY <= minX && minY <= minZ)
        {
            float dir = (playerCenter.y < cubeCenter.y) ? -1.0f : 1.0f;
            position = XMVectorSetY(position, XMVectorGetY(position) + dir * (minY + skin));
            velocity = XMVectorSetY(velocity, 0.0f);
            if (dir > 0.0f) g_IsJump = false;
        }
        else if (minX <= minY && minX <= minZ)
        {
            float dir = (playerCenter.x < cubeCenter.x) ? -1.0f : 1.0f;
            position = XMVectorSetX(position, XMVectorGetX(position) + dir * (minX + skin));
            velocity = XMVectorSetX(velocity, 0.0f);
        }
        else
        {
            float dir = (playerCenter.z < cubeCenter.z) ? -1.0f : 1.0f;
            position = XMVectorSetZ(position, XMVectorGetZ(position) + dir * (minZ + skin));
            velocity = XMVectorSetZ(velocity, 0.0f);
        }
    }

    // 7. GROUND COLLISION (Always applied)
    if (XMVectorGetY(position) < 0.0f)
    {
        position = XMVectorSetY(position, 0.0f);
        velocity = XMVectorSetY(velocity, 0.0f);
        g_IsJump = false;
    }

    // store intermediate result
    XMStoreFloat3(&g_PlayerPosition, position);
    XMStoreFloat3(&g_PlayerVelocity, velocity);

    // 8. Extra side-correction (keep, but uses the already-corrected position/velocity)
    if (hit.isHit) {
        if (hit.normal.x > 0.0f) {
            position = XMVectorSetX(position, cube.max.x + 1.0f);
            // zero X velocity
            velocity = velocity * XMVECTOR{ 0.0f, 1.0f, 1.0f };
            XMStoreFloat3(&g_PlayerVelocity, velocity);
        }
        else if (hit.normal.x < 0.0f) {
            position = XMVectorSetX(position, cube.min.x - 1.0f);
            velocity = velocity * XMVECTOR{ 0.0f, 1.0f, 1.0f };
            XMStoreFloat3(&g_PlayerVelocity, velocity);
        }
    }

    XMStoreFloat3(&g_PlayerPosition, position);
    XMStoreFloat3(&g_PlayerVelocity, velocity);
}

void Player_Draw()
{
    Light_SetSpecularWorld(Camera_GetPosition(), 4.0f, { 0.3f, 0.25f, 0.2f, 1.0f });

    float angle = -atan2f(g_PlayerFront.z, g_PlayerFront.x) + XMConvertToRadians(270.0f);
    XMMATRIX rotation = XMMatrixRotationY(angle);
    XMMATRIX translation = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 1.0f, g_PlayerPosition.z);
    XMMATRIX world = rotation * translation;

    ModelDraw(g_pPlayerModel, world);
}

const XMFLOAT3& Player_GetPosition()
{
    return g_PlayerPosition;
}

const XMFLOAT3& Player_GetFront()
{
    return g_PlayerFront;
}

AABB Player_GetAABB()
{
    return {
        { g_PlayerPosition.x - 1.0f, g_PlayerPosition.y,       g_PlayerPosition.z - 1.0f },
        { g_PlayerPosition.x + 1.0f, g_PlayerPosition.y + 2.0f, g_PlayerPosition.z + 1.0f }
    };
}
