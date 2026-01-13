/*========================================================================================
    Enemy Humanoid [enemy_humanoid.h]                        PYAE SONE THANT
                                                             DATE:26/11/2025
=========================================================================================*/
#ifndef ENEMY_HUMANOID_H
#define ENEMY_HUMANOID_H

#include "enemy.h"
#include <DirectXMath.h>
#include "collision.h"
#include "sprite_anim.h"



void Fireball_Update(double elapsed);

class EnemyHumanoid : public Enemy
{
protected:
    // ---------------- BASIC DATA ----------------
    DirectX::XMFLOAT3 m_Position{ 0,0,0 };
    DirectX::XMFLOAT3 m_VisualOffset{ 0.0f, 0.6f, 0.0f }; // Base offset
    DirectX::XMFLOAT2 m_VisualScale{ 2.0f, 2.0f };      // Default scale for all enemies

    float m_DetectionRadius{ 5.0f };
    float m_WalkSpeed{ 1.5f };
    float m_ChaseSpeed{ 4.0f };

    int   m_Hp{ 100 };
    bool  m_IsDead{ false };
    bool  m_FacingRight{ true };


    double m_HitCooldown{ 0.0 };
    double m_AttackCooldown{ 0.0 };
    double m_NoAttackTimer{ 0.0 };

    // ---------------- ANIMATION PLAYERS ----------------
    int m_AnimLeftPlayId{ -1 };
    int m_AnimRightPlayId{ -1 };
    int m_AnimLeftChasePlayId{ -1 };
    int m_AnimRightChasePlayId{ -1 };
    int m_AnimLeftIdlePlayId{ -1 };
    int m_AnimRightIdlePlayId{ -1 };
    int m_AnimDeathPlayId{ -1 };
    int m_AnimHitLeftPlayId{ -1 };
    int m_AnimHitRightPlayId{ -1 };

    //attack
    int m_AnimLeftAttackPlayId{ -1 };
    int m_AnimRightAttackPlayId{ -1 };

    bool m_IsInvincible{ false }; // New flag for the attack duration

protected:
    EnemyHumanoid(const DirectX::XMFLOAT3& position);

public:
    void Update(double elapsed_time) override;
    void Damage(int damage) override;

    bool IsDestroy() const override;

    Sphere GetCollision() const override;
    AABB   GetAABB() const override;

protected:
    // ================= STATES =================

    class StatePatrol : public Enemy::State
    {
    public:
        bool m_MovingRight{ true };

    protected:
        EnemyHumanoid* m_pOwner{};
        float m_StartX{};

    public:
        StatePatrol(EnemyHumanoid* owner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    class StateChase : public Enemy::State
    {
    protected:
        EnemyHumanoid* m_pOwner{};
        double m_GiveUpTimer{};

    public:
        StateChase(EnemyHumanoid* owner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    class StateIdle : public Enemy::State
    {
    protected:
        EnemyHumanoid* m_pOwner{};
        bool m_FacingRight{};
        bool m_Indefinite{};
        float m_Time{};

    public:
        StateIdle(EnemyHumanoid* owner, bool facingRight, bool indefinite);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    class StateHit : public Enemy::State
    {
    public:
        enum class Prev { PATROL, CHASE, IDLE };

    protected:
        EnemyHumanoid* m_pOwner{};
        Prev m_Prev{};
        bool m_FacingRight{};

    public:
        StateHit(EnemyHumanoid* owner, Prev prev, bool facingRight);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    class StateDeath : public Enemy::State
    {
    protected:
        EnemyHumanoid* m_pOwner{};
        bool m_CoinDropped{ false };

    public:
        StateDeath(EnemyHumanoid* owner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
    class StateAttack : public Enemy::State
    {
    protected:
        EnemyHumanoid* m_pOwner{};
        bool m_FacingRight{};
        bool m_HitDone{ false };
    public:
        StateAttack(EnemyHumanoid* owner, bool facingRight);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
    class StateShoot : public Enemy::State
    {
    protected:
        EnemyHumanoid* m_pOwner{};
        double m_Cooldown{};       // Time between shots
        bool m_FacingRight{};

    public:
        StateShoot(EnemyHumanoid* owner, bool facingRight);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};
void Fireball_Draw();
#endif // ENEMY_HUMANOID_H
