/*========================================================================================


    Enemy Normal [enemy.h]										        PYAE SONE THANT
                                                                        DATE:12/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef ENEMY_NORMAL_H
#define ENEMY_NORMAL_H

#include "enemy.h"
#include <DirectXMath.h>
#include "texture.h"
#include "sprite_anim.h"


class EnemyNormal : public Enemy
{
private:
    DirectX::XMFLOAT3 m_Position{ 0.0f, 0.0f, 0.0f };
    float m_DetectionRadius{ 2.0f };
    int m_Hp{ 500 };
    bool m_FacingRight{ true };
    DirectX::XMFLOAT3 m_VisualOffset{ 0.0f, 0.6f, 0.0f };

    //Animation Player IDs Animation Play IDs
    int m_AnimLeftPlayId{ -1 };
    int m_AnimRightPlayId{ -1 };
    int m_AnimLeftChasePlayId{ -1 };
    int m_AnimRightChasePlayId{ -1 };
    int m_AnimLeftIdlePlayId{ -1 };
    int m_AnimRightIdlePlayId{ -1 };
    int m_AnimDeathPlayId{ -1 };

    //Animation Pattern IDs
    int m_AnimLeftId{ -1 };
    int m_AnimRightId{ -1 };
    int m_AnimLeftChaseId{ -1 };
    int m_AnimRightChaseId{ -1 };
    int m_AnimLeftIdleId{ -1 };
    int m_AnimRightIdleId{ -1 };
    int m_AnimDeathId{ -1 };

    // Texture IDs
    int m_TexWhiteId{ -1 };
    int m_TexRedId{ -1 };
    int m_TexLeftWalkId{ -1 };
    int m_TexRightWalkId{ -1 };
    int m_TexLeftIdleId{ -1 };
    int m_TexRightIdleId{ -1 };
    int m_TexDeathId{ -1 };
    int m_TexHitLeftId{ -1 };
    int m_TexHitRightId{ -1 };

    //getting hit animations test
    int m_AnimHitLeftId{ -1 };
    int m_AnimHitRightId{ -1 };
    int m_AnimHitLeftPlayId{ -1 };
    int m_AnimHitRightPlayId{ -1 };


public:
    EnemyNormal(const DirectX::XMFLOAT3& position);

    bool m_IsDead{ false };

    void Damage(int damage);

    void Update(double elapsed_time);

    bool IsDestroy() const override
    {
        return m_IsDead && dynamic_cast<EnemyNormalStateDeath*>(GetState()) == nullptr;
    }
    Sphere GetCollision() const override
    {
        return { { m_Position.x, m_Position.y + 0.5f, m_Position.z }, 0.5f };
    }

    AABB GetAABB() const override
    {
        return { { m_Position.x - 0.3f, m_Position.y, m_Position.z - 0.3f },
                 { m_Position.x + 0.3f, m_Position.y + 1.0f, m_Position.z + 0.3f } };
    }

private:
    double m_HitCooldown{ 0.0 };
    // ------------------- PATROL -------------------
    class EnemyNormalStatePatrol : public Enemy::State
    {
    public:
        bool m_MovingRight{ true };

    private:
        EnemyNormal* m_pOwner{ nullptr };
        float m_PointX{ 0.0f };

    public:
        EnemyNormalStatePatrol(EnemyNormal* pOwner)
            : m_pOwner(pOwner), m_PointX(pOwner->m_Position.x) {
        }

        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // ------------------- CHASE -------------------
    class EnemyNormalStateChase : public Enemy::State
    {
    private:
        EnemyNormal* m_pOwner{ nullptr };
        double m_GiveUpTimer{ 0.0 };

    public:
        EnemyNormalStateChase(EnemyNormal* pOwner)
            : m_pOwner(pOwner), m_GiveUpTimer(0.0) {
        }

        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // ------------------- IDLE -------------------
    class EnemyNormalStateIdle : public Enemy::State
    {
    private:
        EnemyNormal* m_pOwner{ nullptr };
        float m_AccumulatedTime{ 0.0f };
        bool m_FacingRight{ true };
        bool m_Indefinite{ false };

    public:
        EnemyNormalStateIdle(EnemyNormal* pOwner, bool facingRight, bool indefinite = false)
            : m_pOwner(pOwner), m_FacingRight(facingRight), m_Indefinite(indefinite), m_AccumulatedTime(0.0f) {
        }

        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // ... inside class EnemyNormal
    class EnemyNormalStateHit : public Enemy::State
    {
    public:
        // *** ADDED ENUM TO TRACK PREVIOUS STATE TYPE ***
        enum class PreviousStateType { PATROL, CHASE, IDLE };

    private:
        EnemyNormal* m_pOwner{ nullptr };
        PreviousStateType m_PreviousStateType; // Store the type instead of the pointer
        bool m_PreviousFacingRight; // Store facing direction needed for Patrol/Idle
        float m_AccumulatedTime{ 0.0f };

    public:
        // *** MODIFIED CONSTRUCTOR ***
        EnemyNormalStateHit(EnemyNormal* pOwner, PreviousStateType prevState, bool facingRight)
            : m_pOwner(pOwner), m_PreviousStateType(prevState), m_PreviousFacingRight(facingRight), m_AccumulatedTime(0.0f) {
        }

        void Update(double elapsed_time) override;
        void Draw() const override;
    };
    // ...
    // ------------------- DEATH -------------------
    class EnemyNormalStateDeath : public Enemy::State
    {
    private:
        EnemyNormal* m_pOwner{ nullptr };
        float m_AccumulatedTime{ 0.0f };

    public:
        EnemyNormalStateDeath(EnemyNormal* pOwner)
            : m_pOwner(pOwner), m_AccumulatedTime(0.0f) {
        }

        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // ENEMY_NORMAL_H