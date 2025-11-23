/*===============================================================================

  player cpp[player.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/10/31
---------------------------------------------------------------------------------

=================================================================================*/
#include "player.h"
#include <DirectXMath.h>
#include "key_logger.h"
#include "camera.h"
#include "player_camera.h"
#include "light.h"
#include "map.h"
#include "cube.h"
#include "bullet.h"
#include "texture.h"
#include "sprite_anim.h"
#include "direct3d.h"
#include "sampler.h"

using namespace DirectX;

static XMFLOAT3 g_PlayerPosition{ 0.0f, 0.0f, 0.0f };
static XMFLOAT3 g_PlayerFront{ 0.0f, 0.0f, 1.0f };
static XMFLOAT3 g_PlayerVelocity{};
static XMFLOAT3 g_VisualOffset{ 0.5f, 0.1f, 0.0f }; // tweak Y to center the sprite
static bool g_IsJump = false;
const float PLAYER_SPEED = 8.0f;

//for player sprite animation
static int animPatternUp = -1;
static int animPatternDown = -1;
static int animPatternLeft = -1;
static int animPatternRight = -1;

static int animIdleUp = -1;
static int animIdleDown = -1;
static int animIdleLeft = -1;
static int animIdleRight = -1;

static int animPlayerId = -1;
static int animCurrent = -1;

void Player_Initialize(const XMFLOAT3& position, const XMFLOAT3& front)
{
    g_PlayerPosition = position;
    XMStoreFloat3(&g_PlayerFront, XMVector3Normalize(XMLoadFloat3(&front)));
    g_IsJump = false;

    int frameCount = 8;
    int hPatternMax = 8;
    float secondsPerPattern = 0.1f;
    float Idle_SecondsPerPattern = 0.2f;
    DirectX::XMUINT2 patternSize = { 96, 80 };
    DirectX::XMUINT2 startPos = { 0, 0 };
    bool loop = true;

    int texRunUp = Texture_Load (L"Texture/run_up.png");
    int texRunDown = Texture_Load(L"Texture/run_down.png");
    int texRunLeft = Texture_Load(L"Texture/run_left.png");
    int texRunRight = Texture_Load(L"Texture/run_right.png");

    int texIdleUp = Texture_Load(L"Texture/idle_up.png");
    int texIdleDown = Texture_Load(L"Texture/idle_down.png");
    int texIdleLeft = Texture_Load(L"Texture/idle_left.png");
    int texIdleRight = Texture_Load(L"Texture/idle_right.png");

    animPatternUp    = SpriteAnim_RegisterPattern(texRunUp   ,frameCount , hPatternMax, secondsPerPattern, patternSize, startPos, loop);
    animPatternDown  = SpriteAnim_RegisterPattern(texRunDown , frameCount, hPatternMax, secondsPerPattern, patternSize, startPos, loop);
    animPatternLeft  = SpriteAnim_RegisterPattern(texRunLeft , frameCount, hPatternMax, secondsPerPattern, patternSize, startPos, loop);
    animPatternRight = SpriteAnim_RegisterPattern(texRunRight, frameCount, hPatternMax, secondsPerPattern, patternSize, startPos, loop);

    animIdleUp    = SpriteAnim_RegisterPattern(texIdleUp    , frameCount, hPatternMax, Idle_SecondsPerPattern, patternSize, startPos, loop);
    animIdleDown  = SpriteAnim_RegisterPattern(texIdleDown  , frameCount, hPatternMax, Idle_SecondsPerPattern, patternSize, startPos, loop);
    animIdleLeft  = SpriteAnim_RegisterPattern(texIdleLeft  , frameCount, hPatternMax, Idle_SecondsPerPattern, patternSize, startPos, loop);
    animIdleRight = SpriteAnim_RegisterPattern(texIdleRight , frameCount, hPatternMax, Idle_SecondsPerPattern, patternSize, startPos, loop);

    // Start with idle facing DOWN
    animPlayerId = SpriteAnim_CreatePlayer(animIdleDown);
    animCurrent = animIdleDown;

}

void Player_Finalize()
{
    if (animPlayerId >= 0)
        SpriteAnim_DestroyPlayer(animPlayerId);
}

void Player_Update(double elapsed_time)
{
    XMVECTOR old_position = XMLoadFloat3(&g_PlayerPosition);
    XMVECTOR velocity = XMLoadFloat3(&g_PlayerVelocity);
    AABB old_player_aabb = Player_GetAABB();
    XMVECTOR position = old_position;

        // --- Jump input ---
        if (KeyLogger_IsTrigger(KK_SPACE) && !g_IsJump)
        {
            velocity += { 0.0f, 40.0f, 0.0f };
            g_IsJump = true;
        }

    // --- Gravity ---
    XMVECTOR gdir{ 0.0f, 1.0f, 0.0f };
    velocity += gdir * -9.8f * 15.0f * (float)elapsed_time;

    // --- Movement input ---
    XMVECTOR direction{};
    XMVECTOR forward = { 0.0f, 0.0f, 1.0f };
    XMVECTOR right = { 1.0f, 0.0f, 0.0f };
    bool moving = false;

    if (KeyLogger_IsPressed(KK_W)) { direction += forward; moving = true; }
    if (KeyLogger_IsPressed(KK_S)) { direction -= forward; moving = true; }
    if (KeyLogger_IsPressed(KK_A)) { direction -= right;   moving = true; }
    if (KeyLogger_IsPressed(KK_D)) { direction += right;   moving = true; }

    const float FRICTION = 5.0f; // adjust: higher = stops faster, lower = more slippery

    if (moving)
    {
        direction = XMVector3Normalize(direction);
        XMStoreFloat3(&g_PlayerFront, direction);

        // Accelerate towards input direction
        XMVECTOR targetVel = direction * PLAYER_SPEED;
        velocity = XMVectorLerp(velocity, targetVel, 0.2f); // smooth transition (slight slipperiness)
    }
    else
    {
        // Apply friction to slow down gradually
        velocity = XMVectorLerp(velocity, XMVectorSetX(velocity, 0.0f), 0.1f); // X
        velocity = XMVectorLerp(velocity, XMVectorSetZ(velocity, 0.0f), 0.1f); // Z
    }

    // --- Animation switch ---
    int newAnim = animCurrent;
    if (moving)
    {
        float dx = XMVectorGetX(direction);
        float dz = XMVectorGetZ(direction);

        if (dx > 0.1f)       newAnim = animPatternRight;
        else if (dx < -0.1f) newAnim = animPatternLeft;
        else if (dz > 0.1f)  newAnim = animPatternUp;
        else if (dz < -0.1f) newAnim = animPatternDown;
    }
    else // idle
    {
        // Choose idle animation based on last movement direction
        float fx = g_PlayerFront.x;
        float fz = g_PlayerFront.z;

        if (fz > 0.5f)       newAnim = animIdleUp;
        else if (fz < -0.5f) newAnim = animIdleDown;
        else if (fx < -0.5f) newAnim = animIdleLeft;
        else if (fx > 0.5f)  newAnim = animIdleRight;
    }

    if (newAnim != animCurrent)
    {
        SpriteAnim_DestroyPlayer(animPlayerId);
        animPlayerId = SpriteAnim_CreatePlayer(newAnim);
        animCurrent = newAnim;
    }

    // --- Final physics ---
    velocity += -velocity * (float)(4.0f * elapsed_time);
    position += velocity * (float)elapsed_time;

    // --- Collision with cubes ---
    for (int i = 0; i < Map_GetObjectCount(); ++i)
    {
        AABB cube = Cube_GetAABB(Map_GetObject(i)->Position);
        XMStoreFloat3(&g_PlayerPosition, position);
        AABB player = Player_GetAABB();
        Hit hit = Collision_IsHitAABB(cube, player);

        if (hit.isHit)
        {
            const float skin = 0.002f;

            float overlapX1 = cube.max.x - player.min.x;
            float overlapX2 = player.max.x - cube.min.x;
            float overlapY1 = cube.max.y - player.min.y;
            float overlapY2 = player.max.y - cube.min.y;
            float overlapZ1 = cube.max.z - player.min.z;
            float overlapZ2 = player.max.z - cube.min.z;

            float minX = (overlapX1 < overlapX2) ? overlapX1 : overlapX2;
            float minY = (overlapY1 < overlapY2) ? overlapY1 : overlapY2;
            float minZ = (overlapZ1 < overlapZ2) ? overlapZ1 : overlapZ2;

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

        XMStoreFloat3(&g_PlayerPosition, position);
    }

    //Ground collision
    if (XMVectorGetY(position) < 0.0f)
    {
        position = XMVectorSetY(position, 0.0f);
        velocity = XMVectorSetY(velocity, 0.0f);
        g_IsJump = false;
    }

    XMStoreFloat3(&g_PlayerPosition, position);
    XMStoreFloat3(&g_PlayerVelocity, velocity);

    //Fire bullet
    if (KeyLogger_IsTrigger(KK_J))
    {
        XMFLOAT3 bulletVel;
        XMStoreFloat3(&bulletVel, XMLoadFloat3(&g_PlayerFront) * 25.0f);

        XMFLOAT3 bulletPos = {
            g_PlayerPosition.x,
            g_PlayerPosition.y + 1.5f,
            g_PlayerPosition.z
        };

        Bullet_Create(bulletPos, bulletVel);
    }
 
}

void Player_Draw()
{
    if (animPlayerId >= 0)
    {
        Direct3D_SetAlphaBlendState();
        Direct3D_SetDepthReadOnly(true);
        Sampler_SetFilterPoint();

            XMFLOAT3 drawPos = {
                g_PlayerPosition.x + g_VisualOffset.x,
                g_PlayerPosition.y + g_VisualOffset.y,
                g_PlayerPosition.z + g_VisualOffset.z
        };

        BillboardAnim_Draw(animPlayerId, drawPos, { 1.5f, 1.5f }, { 0.5f, 0.5f });

        Direct3D_SetDefaultBlendState();
        Direct3D_SetDepthEnable(true);
    }

}

const XMFLOAT3& Player_GetPosition() { return g_PlayerPosition; }
const XMFLOAT3& Player_GetFront() { return g_PlayerFront; }

AABB Player_GetAABB()
{
    return {
    { g_PlayerPosition.x - 0.25f, g_PlayerPosition.y,        g_PlayerPosition.z - 0.25f },
    { g_PlayerPosition.x + 0.25f, g_PlayerPosition.y + 1.2f,  g_PlayerPosition.z + 0.25f }
    };
}

AABB Player_ConvertPositionToAABB(const XMVECTOR& position)
{
    AABB aabb;
    XMStoreFloat3(&aabb.min, position - XMVECTOR{ 1.0f, 0.0f, 1.0f });
    XMStoreFloat3(&aabb.max, position + XMVECTOR{ 1.0f, 2.0f, 1.0f });
    return aabb;
}
