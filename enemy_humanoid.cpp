/*========================================================================================
    Enemy Humanoid [enemy_humanoid.cpp]                      PYAE SONE THANT
                                                             DATE:26/11/2025
=========================================================================================*/
#include "enemy_humanoid.h"
#include "player.h"
#include "billboard.h"
#include "direct3d.h"
#include "sampler.h"
#include "map.h"
#include "terrain.h"
#include <cmath>

using namespace DirectX;

static constexpr double HIT_COOLDOWN_TIME = 0.3;
static constexpr float ARRIVE_DISTANCE = 1.2f;


// ----------------------------------------------------------------
EnemyHumanoid::EnemyHumanoid(const XMFLOAT3& position)
    : m_Position(position)
{
}

// ----------------------------------------------------------------
void EnemyHumanoid::Update(double elapsed_time)
{
    if (m_HitCooldown > 0.0)
        m_HitCooldown -= elapsed_time;

    if (GetState())
        GetState()->Update(elapsed_time);
}

// ----------------------------------------------------------------
void EnemyHumanoid::Damage(int damage)
{
    if (m_IsDead || m_HitCooldown > 0.0)
        return;

    m_HitCooldown = HIT_COOLDOWN_TIME;
    m_Hp -= damage;

    if (m_Hp <= 0)
    {
        m_IsDead = true;
        ChangeState(new StateDeath(this));
        return;
    }

    StateHit::Prev prev = StateHit::Prev::IDLE;

    if (dynamic_cast<StatePatrol*>(GetState())) prev = StateHit::Prev::PATROL;
    else if (dynamic_cast<StateChase*>(GetState())) prev = StateHit::Prev::CHASE;

    ChangeState(new StateHit(this, prev, m_FacingRight));
}

// ----------------------------------------------------------------
bool EnemyHumanoid::IsDestroy() const
{
    return m_IsDead && GetState() == nullptr;
}

// ----------------------------------------------------------------
Sphere EnemyHumanoid::GetCollision() const
{
    return { { m_Position.x, m_Position.y + 0.5f, m_Position.z }, 0.5f };
}

// ----------------------------------------------------------------
AABB EnemyHumanoid::GetAABB() const
{
    return {
        { m_Position.x - 0.3f, m_Position.y,     m_Position.z - 0.3f },
        { m_Position.x + 0.3f, m_Position.y + 1, m_Position.z + 0.3f }
    };
}

// ===================== PATROL =====================
EnemyHumanoid::StatePatrol::StatePatrol(EnemyHumanoid* owner)
    : m_pOwner(owner), m_StartX(owner->m_Position.x)
{
}

void EnemyHumanoid::StatePatrol::Update(double elapsed)
{
    float speed = m_pOwner->m_WalkSpeed;
    m_pOwner->m_Position.x += (m_MovingRight ? speed : -speed) * (float)elapsed;
    m_pOwner->m_FacingRight = m_MovingRight;

    if (fabs(m_pOwner->m_Position.x - m_StartX) > 6.0f)
        m_pOwner->ChangeState(new StateIdle(m_pOwner, m_MovingRight, false));

    m_pOwner->m_Position.y =
        Mesh_GetHeightAt(m_pOwner->m_Position.x, m_pOwner->m_Position.z);

    if (Collision_IsOverlapSphere(
        { m_pOwner->m_Position, m_pOwner->m_DetectionRadius },
        Player_GetPosition()))
    {
        m_pOwner->ChangeState(new StateChase(m_pOwner));
    }
}

void EnemyHumanoid::StatePatrol::Draw() const
{
    int anim = m_pOwner->m_FacingRight
        ? m_pOwner->m_AnimRightPlayId
        : m_pOwner->m_AnimLeftPlayId;

    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y;

    BillboardAnim_Draw(anim, pos, { 2,2 }, { 0.5f,1 });
}

EnemyHumanoid::StateChase::StateChase(EnemyHumanoid* owner)
    : m_pOwner(owner), m_GiveUpTimer(0.0)
{
    int anim = m_pOwner->m_FacingRight
        ? m_pOwner->m_AnimRightChasePlayId
        : m_pOwner->m_AnimLeftChasePlayId;

    SpriteAnim_SetFrame(anim, 0);
    SpriteAnim_Resume(anim);
}


void EnemyHumanoid::StateChase::Update(double elapsed)
{
    int anim = m_pOwner->m_FacingRight
        ? m_pOwner->m_AnimRightChasePlayId
        : m_pOwner->m_AnimLeftChasePlayId;

    SpriteAnim_UpdatePlayer(anim, elapsed);

    XMFLOAT3 player = Player_GetPosition();
    XMFLOAT3& pos = m_pOwner->m_Position;

    float dx = player.x - pos.x;
    float dz = player.z - pos.z;
    float dist = sqrtf(dx * dx + dz * dz);

    // Stop moving when close enough
    if (dist > ARRIVE_DISTANCE)
    {
        pos.x += (dx / dist) * m_pOwner->m_ChaseSpeed * (float)elapsed;
        pos.z += (dz / dist) * m_pOwner->m_ChaseSpeed * (float)elapsed;
    }
    else
    {
        // Arrived near player ¨ idle but keep watching
        m_pOwner->ChangeState(
            new StateIdle(m_pOwner, m_pOwner->m_FacingRight, true));
        return;
    }


    pos.y = Mesh_GetHeightAt(pos.x, pos.z);
    m_pOwner->m_FacingRight = dx > 0;

    if (dist > m_pOwner->m_DetectionRadius * 1.5f)
    {
        m_pOwner->ChangeState(
            new StateIdle(m_pOwner, m_pOwner->m_FacingRight, false));
    }
}


void EnemyHumanoid::StateChase::Draw() const
{
    int anim = m_pOwner->m_FacingRight
        ? m_pOwner->m_AnimRightChasePlayId
        : m_pOwner->m_AnimLeftChasePlayId;

    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y;

    BillboardAnim_Draw(anim, pos, { 2,2 }, { 0.5f,1 });
}

// ===================== IDLE =====================
EnemyHumanoid::StateIdle::StateIdle(EnemyHumanoid* owner, bool facing, bool indef)
    : m_pOwner(owner), m_FacingRight(facing), m_Indefinite(indef)
{
}

void EnemyHumanoid::StateIdle::Update(double elapsed)
{
    m_Time += (float)elapsed;

    XMFLOAT3 player = Player_GetPosition();
    XMFLOAT3& pos = m_pOwner->m_Position;

    float dx = player.x - pos.x;
    float dz = player.z - pos.z;
    float dist = sqrtf(dx * dx + dz * dz);

    // Always face player while idling
    m_FacingRight = dx > 0;

    // If player moves away ¨ chase again
    if (m_Indefinite && dist > ARRIVE_DISTANCE &&
        dist <= m_pOwner->m_DetectionRadius)
    {
        m_pOwner->ChangeState(new StateChase(m_pOwner));
        return;
    }

    // If player completely escapes ¨ patrol
    if (m_Indefinite &&
        dist > m_pOwner->m_DetectionRadius * 1.5f)
    {
        m_pOwner->ChangeState(
            new StateIdle(m_pOwner, m_FacingRight, false));
        return;
    }

    // Normal idle ¨ patrol after delay
    if (!m_Indefinite && m_Time > 2.0f)
    {
        StatePatrol* p = new StatePatrol(m_pOwner);
        p->m_MovingRight = !m_FacingRight;
        m_pOwner->ChangeState(p);
    }
}


void EnemyHumanoid::StateIdle::Draw() const
{
    int anim = m_FacingRight
        ? m_pOwner->m_AnimRightIdlePlayId
        : m_pOwner->m_AnimLeftIdlePlayId;

    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y;

    BillboardAnim_Draw(anim, pos, { 2,2 }, { 0.5f,1 });
}

// ===================== HIT =====================
EnemyHumanoid::StateHit::StateHit(
    EnemyHumanoid* owner, Prev prev, bool facing)
    : m_pOwner(owner), m_Prev(prev), m_FacingRight(facing)
{
    int anim = m_FacingRight
        ? m_pOwner->m_AnimHitRightPlayId
        : m_pOwner->m_AnimHitLeftPlayId;

    SpriteAnim_SetFrame(anim, 0);
    SpriteAnim_Resume(anim);
}

void EnemyHumanoid::StateHit::Update(double elapsed)
{
    int anim = m_FacingRight
        ? m_pOwner->m_AnimHitRightPlayId
        : m_pOwner->m_AnimHitLeftPlayId;

    SpriteAnim_UpdatePlayer(anim, elapsed);

    if (SpriteAnim_IsStopped(anim))
    {
        if (m_Prev == Prev::CHASE)
            m_pOwner->ChangeState(new StateChase(m_pOwner));
        else
            m_pOwner->ChangeState(
                new StateIdle(m_pOwner, m_FacingRight, true));
    }
}

void EnemyHumanoid::StateHit::Draw() const
{
    int anim = m_FacingRight
        ? m_pOwner->m_AnimHitRightPlayId
        : m_pOwner->m_AnimHitLeftPlayId;

    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y;

    BillboardAnim_Draw(anim, pos, { 2,2 }, { 0.5f,1 });
}

// ===================== DEATH =====================
EnemyHumanoid::StateDeath::StateDeath(EnemyHumanoid* owner)
    : m_pOwner(owner)
{
    SpriteAnim_SetFrame(m_pOwner->m_AnimDeathPlayId, 0);
    SpriteAnim_Resume(m_pOwner->m_AnimDeathPlayId);
}

void EnemyHumanoid::StateDeath::Update(double elapsed)
{
    SpriteAnim_UpdatePlayer(m_pOwner->m_AnimDeathPlayId, elapsed);

    if (SpriteAnim_IsStopped(m_pOwner->m_AnimDeathPlayId))
        m_pOwner->ChangeState(nullptr);
}

void EnemyHumanoid::StateDeath::Draw() const
{
    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y - 0.25f;

    BillboardAnim_Draw(
        m_pOwner->m_AnimDeathPlayId,
        pos,
        { 2.0f, 2.0f },
        { 0.5f,1 });
}
