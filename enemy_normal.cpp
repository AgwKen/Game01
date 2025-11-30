/*========================================================================================


    Enemey Normal cpp [enemy_normal.cpp]								        PYAE SONE THANT
                                                                        DATE:26/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "enemy_normal.h"
#include "collision.h"
#include "player.h"
using namespace DirectX;
#include "cube.h"
#include "shader3d.h"

void EnemyNormal::EnemyNormalStatePatrol::Update(double elapsed_time)
{
    g_AccumulatedTime += static_cast<float>(elapsed_time);

    m_pOwner->m_Position.x =
        m_PointX + sinf(g_AccumulatedTime) * 3.0f;

    if (Collision_IsOverlapSphere(
        { m_pOwner->m_Position, m_pOwner->m_DetectionRadius },
        Player_GetPosition())) {

        m_pOwner->ChangeState(new EnemyNormalStateChase(m_pOwner));
    }
}

void EnemyNormal::EnemyNormalStatePatrol::Draw() const
{
    CUBE_Draw(m_pOwner->m_TexWhiteId,
        XMMatrixTranslation(m_pOwner->m_Position.x, m_pOwner->m_Position.y, m_pOwner->m_Position.z));

}

void EnemyNormal::EnemyNormalStateChase::Update(double elapsed_time)
{
    //where player is located
    XMVECTOR toPlayer = XMLoadFloat3(&Player_GetPosition()) - XMLoadFloat3((&m_pOwner->m_Position));
    toPlayer = XMVector3Normalize(toPlayer);


    //walk to player
    XMVECTOR position = XMLoadFloat3(&m_pOwner->m_Position) + toPlayer * 2.0f * (float)elapsed_time;
    XMStoreFloat3(&m_pOwner->m_Position, position);

    //gives up
    if (!Collision_IsOverlapSphere(
        { m_pOwner->m_Position, m_pOwner->m_DetectionRadius },
        Player_GetPosition())) {

        g_AccumulatedTime += elapsed_time;

        if (g_AccumulatedTime >= 5.0) {
            m_pOwner->ChangeState(new EnemyNormalStatePatrol(m_pOwner));
        }
    }
    else {
        g_AccumulatedTime = 0.0f;
    }

}

void EnemyNormal::EnemyNormalStateChase::Draw() const
{
    CUBE_Draw(m_pOwner->m_TexRedId,
        XMMatrixTranslation(m_pOwner->m_Position.x, m_pOwner->m_Position.y, m_pOwner->m_Position.z));
}
