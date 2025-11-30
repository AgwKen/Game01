/*========================================================================================


    Enemey Normal [enemy_normal.h]								        PYAE SONE THANT
                                                                        DATE:26/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef ENEMY_NORMAL_H
#define	ENEMY_NORMAL_H

#include "enemy.h"
#include <DirectXMath.h>
#include "texture.h"

class EnemyNormal : public Enemy
{
private:
    DirectX::XMFLOAT3 m_Position{};
    float m_DetectionRadius{3.0f};
    int m_Hp{ 50 };
    int m_TexWhiteId{};
    int m_TexRedId{};


public:
    EnemyNormal(const DirectX::XMFLOAT3& position) : m_Position(position) {
        m_TexWhiteId = Texture_Load(L"Texture/white.png");
        m_TexRedId = Texture_Load(L"Texture/red.png");
        ChangeState(new EnemyNormalStatePatrol(this));
    }

    void Damage(int damage) override {
        m_Hp -= damage;
    }

    bool IsDestroy() const override {   
        return m_Hp <= 0;
    }

    Sphere GetCollision() const {
        return{ m_Position,0.5 };
    }

private:
    class EnemyNormalStatePatrol : public Enemy::State
    {
    private:
        EnemyNormal* m_pOwner{};
        float m_PointX{};
        double g_AccumulatedTime{};


    public:
        EnemyNormalStatePatrol(EnemyNormal* pOwner) 
            : m_pOwner(pOwner) 
            , m_PointX(m_pOwner->m_Position.x)
        {
        }
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    class EnemyNormalStateChase : public State
    {
    private:
        EnemyNormal* m_pOwner{};
        double g_AccumulatedTime{};


    public:
        EnemyNormalStateChase(EnemyNormal* pOwner)
            : m_pOwner(pOwner)
        {
        }
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};


#endif // ENEMY_NORMAL_H

