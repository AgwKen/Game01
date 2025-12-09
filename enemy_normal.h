/*========================================================================================
    Enemy Normal [enemy_normal.h]                                        PYAE SONE THANT
                                                                        DATE:26/11/2025
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
    DirectX::XMFLOAT3 m_Position{};
    DirectX::XMFLOAT3 m_Velocity{};
    float m_DetectionRadius{ 1.0f };
    int m_Hp{ 50 };

    int m_TexWhiteId{};
    int m_TexRedId{};

    // Left/Right walking textures and animations
    int m_TexLeftWalkId{};
    int m_TexRightWalkId{};
    int m_AnimLeftId = -1;
    int m_AnimRightId = -1;
    int m_AnimLeftPlayId = -1;
    int m_AnimRightPlayId = -1;

    // Idle textures and animations
    int m_TexLeftIdleId{};
    int m_TexRightIdleId{};
    int m_AnimLeftIdleId = -1;
    int m_AnimRightIdleId = -1;
    int m_AnimLeftIdlePlayId = -1;
    int m_AnimRightIdlePlayId = -1;

    bool m_FacingRight = true;

    // Visual offset (lift enemy slightly)
    DirectX::XMFLOAT3 m_VisualOffset{ 0.0f, 1.0f, 0.0f }; // lift 1 unit above ground

public:
    EnemyNormal(const DirectX::XMFLOAT3& position);

    void Damage(int damage) override { m_Hp -= damage; }
    bool IsDestroy() const override { return m_Hp <= 0; }

    Sphere GetCollision() const override
    {
        float bodyHeight = 1.0f;
        float bodyRadius = 0.5f;
        return { { m_Position.x, m_Position.y + bodyHeight * 0.5f, m_Position.z }, bodyRadius };
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
    public:  // <-- change from private
        bool m_MovingRight{ true }; // patrol direction
    private:
        EnemyNormal* m_pOwner{};
        float m_PointX{};
    public:
        EnemyNormalStatePatrol(EnemyNormal* pOwner)
            : m_pOwner(pOwner), m_PointX(pOwner->m_Position.x), m_MovingRight(true) {
        }
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // ------------------- CHASE -------------------
    class EnemyNormalStateChase : public State
    {
    private:
        EnemyNormal* m_pOwner{};
        double g_AccumulatedTime{};
    public:
        EnemyNormalStateChase(EnemyNormal* pOwner) : m_pOwner(pOwner) {}
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // ------------------- IDLE -------------------
    class EnemyNormalStateIdle : public State
    {
    private:
        EnemyNormal* m_pOwner{};
        double m_AccumulatedTime{};
        bool m_FacingRight{ true };
    public:
        EnemyNormalStateIdle(EnemyNormal* pOwner, bool facingRight)
            : m_pOwner(pOwner), m_FacingRight(facingRight), m_AccumulatedTime(0.0) {
        }

        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // ENEMY_NORMAL_H
