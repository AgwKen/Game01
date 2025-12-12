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
    int m_Hp{ 5000 };
    bool m_FacingRight{ true };
    DirectX::XMFLOAT3 m_VisualOffset{ 0.0f, 1.0f, 0.0f };

	//Animation Player IDs Animation Play IDs
    int m_AnimLeftPlayId{ -1 };
    int m_AnimRightPlayId{ -1 };
    int m_AnimLeftChasePlayId{ -1 };
    int m_AnimRightChasePlayId{ -1 };
    int m_AnimLeftIdlePlayId{ -1 };
    int m_AnimRightIdlePlayId{ -1 };
    int m_AnimDeathPlayId{ -1 };
    int m_AnimHitPlayId{ -1 };

    //Animation Pattern IDs
    int m_AnimLeftId{ -1 };
    int m_AnimRightId{ -1 };
    int m_AnimLeftChaseId{ -1 };
    int m_AnimRightChaseId{ -1 };
    int m_AnimLeftIdleId{ -1 };
    int m_AnimRightIdleId{ -1 };
    int m_AnimDeathId{ -1 };
    int m_AnimHitId{ -1 };

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

    // ------------------- HIT -------------------
// Hit animation pattern IDs for left/right
    int m_AnimHitLeftId{ -1 };
    int m_AnimHitRightId{ -1 };

    // Hit animation player IDs for left/right
    int m_AnimHitLeftPlayId{ -1 };
    int m_AnimHitRightPlayId{ -1 };


public:
    EnemyNormal(const DirectX::XMFLOAT3& position);

    bool m_IsDead{ false };

    void Damage(int damage);

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

    class EnemyNormalStateHit : public Enemy::State
    {
    private:
        EnemyNormal* m_pOwner{ nullptr };
        Enemy::State* m_PreviousState{ nullptr }; // store previous state
        float m_AccumulatedTime{ 0.0f };

    public:
        EnemyNormalStateHit(EnemyNormal* pOwner, Enemy::State* prevState)
            : m_pOwner(pOwner), m_PreviousState(prevState), m_AccumulatedTime(0.0f) {
        }

        void Update(double elapsed_time) override;
        void Draw() const override;
    };
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
