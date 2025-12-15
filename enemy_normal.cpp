/*========================================================================================


    Enemey Normal [enemy_normal.cpp]								    PYAE SONE THANT
                                                                        DATE:12/11/2025

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

static constexpr double HIT_COOLDOWN_TIME = 0.3;



EnemyNormal::EnemyNormal(const XMFLOAT3& position) : m_Position(position)
{
    m_TexWhiteId = Texture_Load(L"Texture/white.png");
    m_TexRedId = Texture_Load(L"Texture/red.png");

    float walkInterval = 0.45f;
    m_TexLeftWalkId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_left_walk.png");
    m_TexRightWalkId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_Right_walk.png");

    m_AnimLeftId = SpriteAnim_RegisterPattern(m_TexLeftWalkId, 10, 5, walkInterval, { 96, 96 }, { 0, 0 }, true);
    m_AnimRightId = SpriteAnim_RegisterPattern(m_TexRightWalkId, 10, 5, walkInterval, { 96, 96 }, { 0, 0 }, true);

    m_AnimLeftPlayId = SpriteAnim_CreatePlayer(m_AnimLeftId);
    m_AnimRightPlayId = SpriteAnim_CreatePlayer(m_AnimRightId);

    float chaseInterval = 0.3f;
    m_AnimLeftChaseId = SpriteAnim_RegisterPattern(m_TexLeftWalkId, 10, 5, chaseInterval, { 96, 96 }, { 0, 0 }, true);
    m_AnimRightChaseId = SpriteAnim_RegisterPattern(m_TexRightWalkId, 10, 5, chaseInterval, { 96, 96 }, { 0, 0 }, true);

    m_AnimLeftChasePlayId = SpriteAnim_CreatePlayer(m_AnimLeftChaseId);
    m_AnimRightChasePlayId = SpriteAnim_CreatePlayer(m_AnimRightChaseId);

    m_TexLeftIdleId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_left_idle.png");
    m_TexRightIdleId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_right_idle.png");

    m_AnimLeftIdleId = SpriteAnim_RegisterPattern(m_TexLeftIdleId, 6, 3, 0.5f, { 96, 96 }, { 0, 0 }, true);
    m_AnimLeftIdlePlayId = SpriteAnim_CreatePlayer(m_AnimLeftIdleId);

    m_AnimRightIdleId = SpriteAnim_RegisterPattern(m_TexRightIdleId, 6, 3, 0.5f, { 96, 96 }, { 0, 0 }, true);
    m_AnimRightIdlePlayId = SpriteAnim_CreatePlayer(m_AnimRightIdleId);

    // Hit animation
    m_TexHitLeftId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_ishit_left.png");
    m_TexHitRightId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_ishit_right.png");

    m_AnimHitLeftId = SpriteAnim_RegisterPattern(m_TexHitLeftId, 7, 7, 0.1f, { 96, 96 }, { 0, 0 }, false);
    m_AnimHitRightId = SpriteAnim_RegisterPattern(m_TexHitRightId, 7, 7, 0.1f, { 96, 96 }, { 0, 0 }, false);

    m_AnimHitLeftPlayId = SpriteAnim_CreatePlayer(m_AnimHitLeftId);
    m_AnimHitRightPlayId = SpriteAnim_CreatePlayer(m_AnimHitRightId);

    // Death animation
    m_TexDeathId = Texture_Load(L"sprites/Frost_Guardian/Frost_guardian_death.png");

    m_AnimDeathId = SpriteAnim_RegisterPattern(m_TexDeathId, 16, 4, 0.3f, { 192,128 }, { 0,0 }, false);

    m_AnimDeathPlayId = SpriteAnim_CreatePlayer(m_AnimDeathId);

    ChangeState(new EnemyNormalStatePatrol(this));
}

void EnemyNormal::Damage(int damage)
{
    if (m_IsDead)
        return;

    if (m_HitCooldown > 0.0)
        return;

    m_HitCooldown = HIT_COOLDOWN_TIME;

    m_Hp -= damage;

    if (m_Hp <= 0)
    {
        m_IsDead = true;
        SpriteAnim_SetFrame(m_AnimDeathPlayId, 0);
        SpriteAnim_Resume(m_AnimDeathPlayId);
        ChangeState(new EnemyNormalStateDeath(this));
        return;
    }

    if (!dynamic_cast<EnemyNormalStateHit*>(GetState()))
    {
        // Determine current state type to safely restore it later ***
        EnemyNormalStateHit::PreviousStateType prevState = EnemyNormalStateHit::PreviousStateType::IDLE;

        if (dynamic_cast<EnemyNormalStatePatrol*>(GetState()))
            prevState = EnemyNormalStateHit::PreviousStateType::PATROL;
        else if (dynamic_cast<EnemyNormalStateChase*>(GetState()))
            prevState = EnemyNormalStateHit::PreviousStateType::CHASE;

        int hitPlayId = m_FacingRight ? m_AnimHitRightPlayId : m_AnimHitLeftPlayId;

        SpriteAnim_Pause(m_AnimLeftPlayId);
        SpriteAnim_Pause(m_AnimRightPlayId);
        SpriteAnim_Pause(m_AnimLeftIdlePlayId);
        SpriteAnim_Pause(m_AnimRightIdlePlayId);
        SpriteAnim_Pause(m_AnimLeftChasePlayId);
        SpriteAnim_Pause(m_AnimRightChasePlayId);

        SpriteAnim_SetFrame(hitPlayId, 0);
        SpriteAnim_Resume(hitPlayId);

        ChangeState(new EnemyNormalStateHit(this, prevState, m_FacingRight));
    }
}

void EnemyNormal::Update(double elapsed)
{
    if (m_HitCooldown > 0.0)
        m_HitCooldown -= elapsed;

    if (GetState())
        GetState()->Update(elapsed);
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
        m_pOwner->ChangeState(new EnemyNormalStateIdle(m_pOwner, true));
    else if (m_pOwner->m_Position.x <= m_PointX - patrolRange)
        m_pOwner->ChangeState(new EnemyNormalStateIdle(m_pOwner, false));

    float groundHeight = Mesh_GetHeightAt(m_pOwner->m_Position.x, m_pOwner->m_Position.z);
    m_pOwner->m_Position.y = groundHeight;

    if (Collision_IsOverlapSphere({ m_pOwner->m_Position, m_pOwner->m_DetectionRadius }, Player_GetPosition()))
        m_pOwner->ChangeState(new EnemyNormalStateChase(m_pOwner));
}

void EnemyNormal::EnemyNormalStatePatrol::Draw() const
{
    int animToDraw = m_pOwner->m_FacingRight ? m_pOwner->m_AnimRightPlayId : m_pOwner->m_AnimLeftPlayId;

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    Sampler_SetFilterPoint();

    XMFLOAT3 drawPos = m_pOwner->m_Position;
    drawPos.x += m_pOwner->m_VisualOffset.x;
    drawPos.y += m_pOwner->m_VisualOffset.y;
    drawPos.z += m_pOwner->m_VisualOffset.z;

    BillboardAnim_Draw(animToDraw, drawPos, { 2.0f, 2.0f }, { 0.5f, 1.0f });

    Direct3D_SetDefaultBlendState();
    Direct3D_SetDepthEnable(true);
}

// ------------------- CHASE -------------------
void EnemyNormal::EnemyNormalStateChase::Update(double elapsed_time)
{
    XMFLOAT3 playerPos = Player_GetPosition();
    XMFLOAT3 myPos = m_pOwner->m_Position;

    XMVECTOR toPlayer = XMVectorSet(
        playerPos.x - myPos.x, 0.0f,
        playerPos.z - myPos.z, 0.0f);
    float distance = XMVectorGetX(XMVector3Length(toPlayer));
    if (distance > 0.01f)
        toPlayer = XMVector3Normalize(toPlayer);

    const float stopDistance = 1.5f;
    const double giveUpTime = 10.0;

    if (distance <= stopDistance)
    {
        m_pOwner->ChangeState(
            new EnemyNormalStateIdle(m_pOwner, playerPos.x > myPos.x, true)
        );
        m_GiveUpTimer = 0.0;
        return;
    }

    float chaseSpeed = 4.0f;
    XMVECTOR newPos =
        XMVectorSet(myPos.x, myPos.y, myPos.z, 0.0f) +
        toPlayer * chaseSpeed * static_cast<float>(elapsed_time);

    XMStoreFloat3(&m_pOwner->m_Position, newPos);

    float groundHeight =
        Mesh_GetHeightAt(m_pOwner->m_Position.x, m_pOwner->m_Position.z);
    m_pOwner->m_Position.y = groundHeight;

    m_pOwner->m_FacingRight = (playerPos.x > myPos.x);

	// Update chase animation again
    int chasePlayId = m_pOwner->m_FacingRight
        ? m_pOwner->m_AnimRightChasePlayId
        : m_pOwner->m_AnimLeftChasePlayId;

    if (SpriteAnim_IsStopped(chasePlayId))
    {
        SpriteAnim_SetFrame(chasePlayId, 0);
        SpriteAnim_Resume(chasePlayId);
    }

    SpriteAnim_UpdatePlayer(chasePlayId, elapsed_time);

    if (!Collision_IsOverlapSphere(
        { m_pOwner->m_Position, m_pOwner->m_DetectionRadius }, playerPos))
    {
        m_GiveUpTimer += elapsed_time;
        if (m_GiveUpTimer >= giveUpTime)
        {
            EnemyNormalStatePatrol* patrolState =
                new EnemyNormalStatePatrol(m_pOwner);
            patrolState->m_MovingRight = !m_pOwner->m_FacingRight;
            m_pOwner->ChangeState(patrolState);
            m_GiveUpTimer = 0.0;
        }
    }
    else
    {
        m_GiveUpTimer = 0.0;
    }
}

void EnemyNormal::EnemyNormalStateChase::Draw() const
{
    int animToDraw = m_pOwner->m_FacingRight ? m_pOwner->m_AnimRightChasePlayId : m_pOwner->m_AnimLeftChasePlayId;

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    Sampler_SetFilterPoint();

    XMFLOAT3 drawPos = m_pOwner->m_Position;
    drawPos.x += m_pOwner->m_VisualOffset.x;
    drawPos.y += m_pOwner->m_VisualOffset.y;
    drawPos.z += m_pOwner->m_VisualOffset.z;

    BillboardAnim_Draw(animToDraw, drawPos, { 2.0f, 2.0f }, { 0.5f, 1.0f });

    Direct3D_SetDefaultBlendState();
    Direct3D_SetDepthEnable(true);
}

// ------------------- IDLE -------------------
void EnemyNormal::EnemyNormalStateIdle::Update(double elapsed_time)
{
    if (m_Indefinite)
    {
        XMFLOAT3 playerPos = Player_GetPosition();
        XMFLOAT3 myPos = m_pOwner->m_Position;

        float distance = sqrtf((playerPos.x - myPos.x) * (playerPos.x - myPos.x) +
            (playerPos.z - myPos.z) * (playerPos.z - myPos.z));
        const float stopDistance = 2.5f;

        if (distance > stopDistance && Collision_IsOverlapSphere({ m_pOwner->m_Position, m_pOwner->m_DetectionRadius }, playerPos))
        {
            m_pOwner->ChangeState(new EnemyNormalStateChase(m_pOwner));
        }
        return;
    }

    m_AccumulatedTime += static_cast<float>(elapsed_time);
    const double giveUpTime = 10.0;

    if (m_AccumulatedTime >= giveUpTime)
    {
        EnemyNormalStatePatrol* patrolState = new EnemyNormalStatePatrol(m_pOwner);
        patrolState->m_MovingRight = !m_FacingRight;
        m_pOwner->ChangeState(patrolState);
    }

    if (Collision_IsOverlapSphere({ m_pOwner->m_Position, m_pOwner->m_DetectionRadius }, Player_GetPosition()))
    {
        m_pOwner->ChangeState(new EnemyNormalStateChase(m_pOwner));
    }
}

void EnemyNormal::EnemyNormalStateIdle::Draw() const
{
    int animToDraw = m_FacingRight ? m_pOwner->m_AnimRightIdlePlayId : m_pOwner->m_AnimLeftIdlePlayId;

    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    Sampler_SetFilterPoint();

    XMFLOAT3 drawPos = m_pOwner->m_Position;
    drawPos.x += m_pOwner->m_VisualOffset.x;
    drawPos.y += m_pOwner->m_VisualOffset.y;
    drawPos.z += m_pOwner->m_VisualOffset.z;

    BillboardAnim_Draw(animToDraw, drawPos, { 2.0f, 2.0f }, { 0.5f, 1.0f });

    Direct3D_SetDefaultBlendState();
    Direct3D_SetDepthEnable(true);
}

// ------------------- DEATH -------------------
void EnemyNormal::EnemyNormalStateDeath::Update(double elapsed_time)
{
    if (!SpriteAnim_IsStopped(m_pOwner->m_AnimDeathPlayId))
    {
        SpriteAnim_UpdatePlayer(m_pOwner->m_AnimDeathPlayId, elapsed_time);
    }

    m_pOwner->m_IsDead = true;
}

void EnemyNormal::EnemyNormalStateDeath::Draw() const
{
    Direct3D_SetAlphaBlendState();
    Direct3D_SetDepthReadOnly(true);
    Sampler_SetFilterPoint();

    XMFLOAT3 drawPos = m_pOwner->m_Position;
    drawPos.x += m_pOwner->m_VisualOffset.x;
    drawPos.y += m_pOwner->m_VisualOffset.y;
    drawPos.z += m_pOwner->m_VisualOffset.z;

    drawPos.y -= 0.23f;// change in scale is need to decrese its Y

    BillboardAnim_Draw(m_pOwner->m_AnimDeathPlayId, drawPos, { 3.5f, 2.5f }, { 0.5f, 1.0f });

    Direct3D_SetDefaultBlendState();
    Direct3D_SetDepthEnable(true);
}


void EnemyNormal::EnemyNormalStateHit::Update(double elapsed_time)
{
    int hitPlayId = m_pOwner->m_FacingRight
        ? m_pOwner->m_AnimHitRightPlayId
        : m_pOwner->m_AnimHitLeftPlayId;

    SpriteAnim_UpdatePlayer(hitPlayId, elapsed_time);

    if (SpriteAnim_IsStopped(hitPlayId))
    {
        SpriteAnim_Pause(hitPlayId);

        switch (m_PreviousStateType)
        {
        case PreviousStateType::PATROL:
        {
            EnemyNormalStatePatrol* patrolState =
                new EnemyNormalStatePatrol(m_pOwner);
            patrolState->m_MovingRight = m_PreviousFacingRight;

            int patrolPlayId = m_PreviousFacingRight
                ? m_pOwner->m_AnimRightPlayId
                : m_pOwner->m_AnimLeftPlayId;

            SpriteAnim_SetFrame(patrolPlayId, 0);
            SpriteAnim_Resume(patrolPlayId);

            m_pOwner->ChangeState(patrolState);
            break;
        }
        case PreviousStateType::CHASE:
        {
            int chasePlayId = m_PreviousFacingRight
                ? m_pOwner->m_AnimRightChasePlayId
                : m_pOwner->m_AnimLeftChasePlayId;

            SpriteAnim_SetFrame(chasePlayId, 0);
            SpriteAnim_Resume(chasePlayId);

            m_pOwner->ChangeState(new EnemyNormalStateChase(m_pOwner));
            break;
        }
        case PreviousStateType::IDLE:
        default:
        {
            int idlePlayId = m_PreviousFacingRight
                ? m_pOwner->m_AnimRightIdlePlayId
                : m_pOwner->m_AnimLeftIdlePlayId;

            SpriteAnim_SetFrame(idlePlayId, 0);
            SpriteAnim_Resume(idlePlayId);

            m_pOwner->ChangeState(
                new EnemyNormalStateIdle(m_pOwner, m_PreviousFacingRight, true)
            );
            break;
        }
        }
    }
}

void EnemyNormal::EnemyNormalStateHit::Draw() const
{
    int animToDraw = m_pOwner->m_FacingRight ? m_pOwner->m_AnimHitRightPlayId : m_pOwner->m_AnimHitLeftPlayId;
    XMFLOAT3 drawPos = m_pOwner->m_Position;
    drawPos.x += m_pOwner->m_VisualOffset.x;
    drawPos.y += m_pOwner->m_VisualOffset.y;
    drawPos.z += m_pOwner->m_VisualOffset.z;

    BillboardAnim_Draw(animToDraw, drawPos, { 2.0f, 2.0f }, { 0.5f, 1.0f });
}