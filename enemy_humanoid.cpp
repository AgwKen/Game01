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
#include "coin.h"
#include <vector>
#include "texture.h"
#include "trajetory3d.h"

using namespace DirectX;

static constexpr double HIT_COOLDOWN_TIME = 0.3;
static constexpr float ATTACK_MOVE_SPEED = 2.0f;
static constexpr float ATTACK_RANGE = 1.0f;
static constexpr float ATTACK_TRIGGER_DISTANCE = 1.5f;
static constexpr float ATTACK_HIT_DISTANCE = 1.5f;


extern std::vector<Coin> g_Coins;
extern int g_PlayerCoinScore; // top-left score


// Simple structure for the moving fireball projectile
struct Fireball {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 direction;
    float speed;
    float active;
};

static std::vector<Fireball> g_Fireballs;

void Fireball_Update(double elapsed) {
    for (auto& f : g_Fireballs) {
        if (!f.active) continue;

        // Move the fireball
        f.position.x += f.direction.x * f.speed * (float)elapsed;
        f.position.z += f.direction.z * f.speed * (float)elapsed;

        // Visual: Create a particle at the current position for the trail
        XMFLOAT4 orange = { 1.0f, 0.4f, 0.0f, 1.0f };
        Trajectory3d_Create(f.position, orange, 0.8f, 0.5);

        // Logic: Simple distance check for player hit
        XMFLOAT3 pPos = Player_GetPosition();
        float dx = pPos.x - f.position.x;
        float dz = pPos.z - f.position.z;
        if (sqrtf(dx * dx + dz * dz) < 0.5f) {
            Player_Damage(10, f.direction, 5.0f);
            f.active = false; // Destroy on hit
        }
    }
}

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

    if (m_AttackCooldown > 0.0)
        m_AttackCooldown -= elapsed_time;

    if (m_NoAttackTimer > 0.0)
        m_NoAttackTimer -= elapsed_time;
    Fireball_Update(elapsed_time);

    if (GetState())
        GetState()->Update(elapsed_time);
}

// ----------------------------------------------------------------
void EnemyHumanoid::Damage(int damage)
{
    if (m_IsDead || m_HitCooldown > 0.0 || m_IsInvincible)
        return;

    m_HitCooldown = HIT_COOLDOWN_TIME;
    m_Hp -= damage;

    m_NoAttackTimer = 0.5; // enemy cannot attack for 1 second after being hit

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

    if (fabs(m_pOwner->m_Position.x - m_StartX) > 1.0f)
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
    if (dist > ATTACK_TRIGGER_DISTANCE)
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

    m_FacingRight = dx > 0;

    if (m_Indefinite &&
        dist <= ATTACK_TRIGGER_DISTANCE &&
        m_pOwner->m_AttackCooldown <= 0.0 &&
        m_pOwner->m_NoAttackTimer <= 0.0)
    {
        // Start the pre-attack timer if it's not already running
        if (m_pOwner->m_PreAttackTimer <= 0.0)
            m_pOwner->m_PreAttackTimer = 2.0; // 3 seconds delay

        // Countdown the timer
        m_pOwner->m_PreAttackTimer -= (float)elapsed;

        // Only attack when timer finishes
        if (m_pOwner->m_PreAttackTimer <= 0.0)
        {
            m_pOwner->ChangeState(new StateAttack(m_pOwner, m_FacingRight));
            return;
        }
    }
    else
    {
        // Reset the timer if player leaves the range
        m_pOwner->m_PreAttackTimer = 0.0;
    }

    // If player moves away ¨ chase again
    if (m_Indefinite && dist > ATTACK_TRIGGER_DISTANCE &&
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
    // Inside EnemyHumanoid::StateIdle::Update
    if (m_Indefinite && dist > 5.0f && dist < 16.0f && m_pOwner->m_AttackCooldown <= 0.0)
    {
        // If player is in "Ranged Zone", switch to shooting state
        m_pOwner->ChangeState(new StateShoot(m_pOwner, m_FacingRight));
        return;
    }
}


void EnemyHumanoid::StateIdle::Draw() const
{
    int anim = m_FacingRight ? m_pOwner->m_AnimRightIdlePlayId : m_pOwner->m_AnimLeftIdlePlayId;

    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y;

    // Use the variable scale instead of { 2, 2 }
    BillboardAnim_Draw(anim, pos, m_pOwner->m_VisualScale, { 0.5f, 1 });
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

    // DROP COIN IMMEDIATELY HERE
    if (!m_CoinDropped)
    {
        Coin coin{};
        coin.position = m_pOwner->m_Position;
        coin.position.y += m_pOwner->m_VisualOffset.y + 0.2f;
        coin.spawnY = coin.position.y;
        coin.timer = 0.0f;
        coin.collected = false;

        int texCoin = Texture_Load(L"Texture/coin.png");
        int animId = SpriteAnim_RegisterPattern(texCoin, 5, 5, 0.1, { 16,16 }, { 0,0 }, true);
        coin.animPlayId = SpriteAnim_CreatePlayer(animId);

        g_Coins.push_back(coin);
        m_CoinDropped = true;
    }
}

void EnemyHumanoid::StateDeath::Update(double elapsed)
{
    // Update death animation
    SpriteAnim_UpdatePlayer(m_pOwner->m_AnimDeathPlayId, elapsed);

    // Only spawn coin and mark for destruction after animation is fully stopped
    if (SpriteAnim_IsStopped(m_pOwner->m_AnimDeathPlayId))
    {
        // Spawn coin **once**
        if (!m_CoinDropped)
        {
            Coin coin{};
            coin.position = m_pOwner->m_Position;
            coin.position.y += m_pOwner->m_VisualOffset.y;
            coin.spawnY = coin.position.y;
            coin.timer = 0.0f;
            coin.collected = false;

            int texCoin = Texture_Load(L"Texture/coin.png");
            int animId = SpriteAnim_RegisterPattern(texCoin, 5, 5, 0.1, { 16,16 }, { 0,0 }, true);
            coin.animPlayId = SpriteAnim_CreatePlayer(animId);

            g_Coins.push_back(coin);

            m_CoinDropped = true;
        }

        // Now mark enemy state for destruction
        m_pOwner->ChangeState(nullptr);
    }
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

// ===================== ATTACK =====================
EnemyHumanoid::StateAttack::StateAttack(EnemyHumanoid* owner, bool facingRight)
    : m_pOwner(owner), m_FacingRight(facingRight)
{
    m_pOwner->m_IsInvincible = true; // Set invincibility
    int anim = m_FacingRight ? m_pOwner->m_AnimRightAttackPlayId : m_pOwner->m_AnimLeftAttackPlayId;

    SpriteAnim_SetFrame(anim, 0);
    SpriteAnim_Resume(anim);
}

void EnemyHumanoid::StateAttack::Update(double elapsed)
{
    int anim = m_FacingRight
        ? m_pOwner->m_AnimRightAttackPlayId
        : m_pOwner->m_AnimLeftAttackPlayId;

    SpriteAnim_UpdatePlayer(anim, elapsed);

    int frame = SpriteAnim_GetCurrentFrame(anim);

    XMFLOAT3& enemyPos = m_pOwner->m_Position;
    XMFLOAT3 playerPos = Player_GetPosition();

    float dx = playerPos.x - enemyPos.x;
    float dz = playerPos.z - enemyPos.z;
    float distSq = dx * dx + dz * dz;
    float dist = sqrtf(distSq);

    // --------------------------------------------------
    // Face player (always)
    // --------------------------------------------------
    m_FacingRight = (dx > 0.0f);
    m_pOwner->m_FacingRight = m_FacingRight;

    // --------------------------------------------------
    // Only move BEFORE swing frames (no sliding hits)
    // --------------------------------------------------
    bool inSwing = (frame >= 4 && frame <= 7);

    if (!inSwing && dist > ATTACK_RANGE && dist > 0.001f)
    {
        float nx = dx / dist;
        float nz = dz / dist;

        enemyPos.x += nx * ATTACK_MOVE_SPEED * (float)elapsed;
        enemyPos.z += nz * ATTACK_MOVE_SPEED * (float)elapsed;

        enemyPos.y = Mesh_GetHeightAt(enemyPos.x, enemyPos.z);
    }

    // --------------------------------------------------
    // Deal damage ONCE during active frames
    // --------------------------------------------------
    if ((frame == 5 || frame == 6) && !m_HitDone)
    {
        const float hitRange = 10.8f; // tighter = fairer
        const float hitRangeSq = hitRange * hitRange;

        if (distSq <= hitRangeSq && dist > 0.001f)
        {
            // Real forward direction (XZ plane)
            float len = sqrtf(dx * dx + dz * dz);
            if (len > 0.001f)
            {
                XMFLOAT3 forward = { dx / len, 0.0f, dz / len };

                // Enemy facing direction (sprite)
                XMFLOAT3 facingDir = m_FacingRight
                    ? XMFLOAT3{ 1.0f, 0.0f, 0.0f }
                : XMFLOAT3{ -1.0f, 0.0f, 0.0f };

                float dot = forward.x * facingDir.x + forward.z * facingDir.z;

                if (dot > 0.3f) // allow slight angle
                {
                    XMFLOAT3 knockDir = { forward.x, 0.0f, forward.z };
                    Player_Damage(20, knockDir, 8.0f);
                    m_HitDone = true;
                }
            }


        }
    }

    // --------------------------------------------------
    // End attack
    // --------------------------------------------------
    if (SpriteAnim_IsStopped(anim))
    {
        m_pOwner->m_IsInvincible = false;
        m_pOwner->m_AttackCooldown = 2.0;
        m_pOwner->ChangeState(new StateIdle(m_pOwner, m_FacingRight, true));
    }
}


void EnemyHumanoid::StateAttack::Draw() const
{
    int anim = m_FacingRight ? m_pOwner->m_AnimRightAttackPlayId : m_pOwner->m_AnimLeftAttackPlayId;
    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y;
    BillboardAnim_Draw(anim, pos, { 2,2 }, { 0.5f,1 });
}

// ===================== SHOOT =====================
EnemyHumanoid::StateShoot::StateShoot(EnemyHumanoid* owner, bool facingRight)
    : m_pOwner(owner), m_FacingRight(facingRight), m_Cooldown(0.0)
{
    // If you have a specific shooting animation ID, set it here
    int anim = m_FacingRight ? m_pOwner->m_AnimRightAttackPlayId : m_pOwner->m_AnimLeftAttackPlayId;
    SpriteAnim_SetFrame(anim, 0);
    SpriteAnim_Resume(anim);
}

void EnemyHumanoid::StateShoot::Update(double elapsed)
{
    // 1. Update the billboard animation
    int anim = m_FacingRight ? m_pOwner->m_AnimRightAttackPlayId : m_pOwner->m_AnimLeftAttackPlayId;
    SpriteAnim_UpdatePlayer(anim, elapsed);

    XMFLOAT3 playerPos = Player_GetPosition();
    XMFLOAT3& enemyPos = m_pOwner->m_Position;

    // 2. Calculate direction to player
    float dx = playerPos.x - enemyPos.x;
    float dz = playerPos.z - enemyPos.z;
    float dist = sqrtf(dx * dx + dz * dz);

    m_pOwner->m_FacingRight = (dx > 0);

    // 3. Spawning Logic on Frame 5
    int frame = SpriteAnim_GetCurrentFrame(anim);
    if (frame == 5 && m_Cooldown <= 0.0)
    {
        XMFLOAT3 spawnPos = enemyPos;
        spawnPos.y += 1.0f; // Height of the hand/staff

        // Add to our projectile list
        Fireball f;
        f.position = spawnPos;
        f.direction = { dx / dist, 0.0f, dz / dist }; // Normalized direction
        f.speed = 10.0f;
        f.active = true;
        g_Fireballs.push_back(f);

        // Visual flash at the start
        Trajectory3d_Create(spawnPos, { 1,1,1,1 }, 1.5f, 0.2);

        m_Cooldown = 1.0;
    }

    // 4. Return to Idle when animation ends
    if (SpriteAnim_IsStopped(anim))
    {
        m_pOwner->m_AttackCooldown = 3.0;
        m_pOwner->ChangeState(new StateIdle(m_pOwner, m_pOwner->m_FacingRight, true));
    }
}

void EnemyHumanoid::StateShoot::Draw() const
{
    int anim = m_pOwner->m_FacingRight ? m_pOwner->m_AnimRightAttackPlayId : m_pOwner->m_AnimLeftAttackPlayId;
    XMFLOAT3 pos = m_pOwner->m_Position;
    pos.y += m_pOwner->m_VisualOffset.y;

    // Draw the humanoid performing the shoot/attack animation
    BillboardAnim_Draw(anim, pos, m_pOwner->m_VisualScale, { 0.5f, 1 });
}

void Fireball_Draw()
{
    for (const auto& f : g_Fireballs) {
        if (!f.active) continue;

        Trajectory3d_Create(f.position, { 1.0f, 0.8f, 0.2f, 1.0f }, 1.2f, 0.01);
    }
}