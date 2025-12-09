/*========================================================================================
    Enemy Normal [enemy_normal.cpp]                                   PYAE SONE THANT
                                                                        DATE:26/11/2025
------------------------------------------------------------------------------------------
=========================================================================================*/
#include "enemy_normal.h"
#include "collision.h"
#include "player.h"
#include "billboard.h"
#include "sprite_anim.h"
#include "direct3d.h"
#include "sampler.h"
#include "map.h"
#include "mesh.h"

using namespace DirectX;

// ------------------- CONSTRUCTOR -------------------
EnemyNormal::EnemyNormal(const XMFLOAT3& position) : m_Position(position)
{
    m_TexWhiteId = Texture_Load(L"Texture/white.png");
    m_TexRedId = Texture_Load(L"Texture/red.png");

    // --- Walk animations ---
    m_TexLeftWalkId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_left_walk.png");
    m_TexRightWalkId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_Right_walk.png");

    m_AnimLeftId = SpriteAnim_RegisterPattern(
        m_TexLeftWalkId,
        10, 5, 0.45, { 96, 96 }, { 0, 0 }, true
    );
    m_AnimLeftPlayId = SpriteAnim_CreatePlayer(m_AnimLeftId);

    m_AnimRightId = SpriteAnim_RegisterPattern(
        m_TexRightWalkId,
        10, 5, 0.45, { 96, 96 }, { 0, 0 }, true
    );
    m_AnimRightPlayId = SpriteAnim_CreatePlayer(m_AnimRightId);

    // --- Idle animations ---
    m_TexLeftIdleId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_left_idle.png");
    m_TexRightIdleId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_right_idle.png");

    m_AnimLeftIdleId = SpriteAnim_RegisterPattern(
        m_TexLeftIdleId,
        6, 3, 0.5, { 96, 96 }, { 0, 0 }, true
    );
    m_AnimLeftIdlePlayId = SpriteAnim_CreatePlayer(m_AnimLeftIdleId);

    m_AnimRightIdleId = SpriteAnim_RegisterPattern(
        m_TexRightIdleId,
        6, 3, 0.5, { 96, 96 }, { 0, 0 }, true
    );
    m_AnimRightIdlePlayId = SpriteAnim_CreatePlayer(m_AnimRightIdleId);

    // Start in patrol state
    ChangeState(new EnemyNormalStatePatrol(this));
}

// ------------------- PATROL -------------------
void EnemyNormal::EnemyNormalStatePatrol::Update(double elapsed_time)
{
    float speed = 1.5f;
    float patrolRange = 6.0f;

    if (m_MovingRight)
        m_pOwner->m_Position.x += speed * static_cast<float>(elapsed_time);
    else
        m_pOwner->m_Position.x -= speed * static_cast<float>(elapsed_time);

    m_pOwner->m_FacingRight = m_MovingRight;

    if (m_pOwner->m_Position.x >= m_PointX + patrolRange)
        m_pOwner->ChangeState(new EnemyNormal::EnemyNormalStateIdle(m_pOwner, true));
    else if (m_pOwner->m_Position.x <= m_PointX - patrolRange)
        m_pOwner->ChangeState(new EnemyNormal::EnemyNormalStateIdle(m_pOwner, false));

    // Keep enemy on terrain
    float groundHeight = Mesh_GetHeightAt(m_pOwner->m_Position.x, m_pOwner->m_Position.z);
    m_pOwner->m_Position.y = groundHeight;

    // Player detection
    if (Collision_IsOverlapSphere({ m_pOwner->m_Position, m_pOwner->m_DetectionRadius }, Player_GetPosition()))
    {
        m_pOwner->ChangeState(new EnemyNormalStateChase(m_pOwner));
    }
}

void EnemyNormal::EnemyNormalStatePatrol::Draw() const
{
    int animToDraw = m_pOwner->m_FacingRight ? m_pOwner->m_AnimRightPlayId : m_pOwner->m_AnimLeftPlayId;

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    Sampler_SetFilterPoint();

    XMFLOAT3 drawPos = {
        m_pOwner->m_Position.x + m_pOwner->m_VisualOffset.x,
        m_pOwner->m_Position.y + m_pOwner->m_VisualOffset.y,
        m_pOwner->m_Position.z + m_pOwner->m_VisualOffset.z
    };

    BillboardAnim_Draw(animToDraw, drawPos, { 2.0f, 2.0f }, { 0.5f, 1.0f });

    Direct3D_SetDefaultBlendState();
    Direct3D_SetDepthEnable(true);
}

// ------------------- CHASE -------------------
void EnemyNormal::EnemyNormalStateChase::Update(double elapsed_time)
{
    XMFLOAT3 playerPos = Player_GetPosition();
    XMFLOAT3 myPos = m_pOwner->m_Position;

    XMVECTOR toPlayer = XMVectorSet(playerPos.x - myPos.x, 0.0f, playerPos.z - myPos.z, 0.0f);
    toPlayer = XMVector3Normalize(toPlayer);

    XMVECTOR newPos = XMVectorSet(myPos.x, myPos.y, myPos.z, 0.0f) + toPlayer * 2.0f * static_cast<float>(elapsed_time);
    XMStoreFloat3(&m_pOwner->m_Position, newPos);

    float groundHeight = Mesh_GetHeightAt(m_pOwner->m_Position.x, m_pOwner->m_Position.z);
    m_pOwner->m_Position.y = groundHeight;

    m_pOwner->m_FacingRight = (playerPos.x > myPos.x);

    if (!Collision_IsOverlapSphere({ m_pOwner->m_Position, m_pOwner->m_DetectionRadius }, playerPos))
    {
        g_AccumulatedTime += elapsed_time;
        if (g_AccumulatedTime >= 5.0)
        {
            m_pOwner->ChangeState(new EnemyNormalStatePatrol(m_pOwner));
        }
    }
    else
    {
        g_AccumulatedTime = 0.0;
    }
}

void EnemyNormal::EnemyNormalStateChase::Draw() const
{
    int animToDraw = m_pOwner->m_FacingRight ? m_pOwner->m_AnimRightPlayId : m_pOwner->m_AnimLeftPlayId;

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    Sampler_SetFilterPoint();

    XMFLOAT3 drawPos = {
        m_pOwner->m_Position.x + m_pOwner->m_VisualOffset.x,
        m_pOwner->m_Position.y + m_pOwner->m_VisualOffset.y,
        m_pOwner->m_Position.z + m_pOwner->m_VisualOffset.z
    };

    BillboardAnim_Draw(animToDraw, drawPos, { 2.0f, 2.0f }, { 0.5f, 1.0f });

    Direct3D_SetDefaultBlendState();
    Direct3D_SetDepthEnable(true);
}

// ------------------- IDLE -------------------
void EnemyNormal::EnemyNormalStateIdle::Update(double elapsed_time)
{
    m_AccumulatedTime += elapsed_time;

    if (m_AccumulatedTime >= 5.0)
    {
        // Switch back to patrol in opposite direction
        EnemyNormal::EnemyNormalStatePatrol* patrolState = new EnemyNormal::EnemyNormalStatePatrol(m_pOwner);
        patrolState->m_MovingRight = !m_FacingRight;
        m_pOwner->ChangeState(patrolState);
    }
}

void EnemyNormal::EnemyNormalStateIdle::Draw() const
{
    int animToDraw = m_FacingRight ? m_pOwner->m_AnimRightIdlePlayId : m_pOwner->m_AnimLeftIdlePlayId;

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    Sampler_SetFilterPoint();

    XMFLOAT3 drawPos = {
        m_pOwner->m_Position.x + m_pOwner->m_VisualOffset.x,
        m_pOwner->m_Position.y + m_pOwner->m_VisualOffset.y,
        m_pOwner->m_Position.z + m_pOwner->m_VisualOffset.z
    };

    BillboardAnim_Draw(animToDraw, drawPos, { 2.0f, 2.0f }, { 0.5f, 1.0f });

    Direct3D_SetDefaultBlendState();
    Direct3D_SetDepthEnable(true);
}
