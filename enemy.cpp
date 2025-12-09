/*========================================================================================


    Enemey cpp [enemy.cpp]										        PYAE SONE THANT
                                                                        DATE:26/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "enemy.h"
#include "enemy_normal.h"

void Enemy::Update(double elapsed_time)
{
    m_pState->Update(elapsed_time);
}
void Enemy::Draw() const
{
    m_pState->Draw();
}

void Enemy::UpdateState()
{
    if (m_pNextState != m_pState) {
        delete m_pState;
        m_pState = m_pNextState;
    }
}

void Enemy::ChangeState(State* pNext)
{
    m_pNextState = pNext;
}

static constexpr int MAX_ENEMY = 16;
static Enemy* g_Enemies[MAX_ENEMY]{};
static int g_EnemyCount{ 0 };

void Enemy_Initialize()
{
    g_EnemyCount = 0;
}

void Enemy_Finalize()
{
    for (int i = 0; i < g_EnemyCount; i++) {
        delete g_Enemies[i];
    }
}

void Enemy_Update(double elapsed_time)
{
    for (int i = 0; i < g_EnemyCount; i++) {
        g_Enemies[i]->UpdateState();
    }

    for (int i = 0; i < g_EnemyCount; i++) {
        g_Enemies[i]->Update(elapsed_time);
    }

    for (int i = g_EnemyCount - 1; i >= 0; i--) {
        if (g_Enemies[i]->IsDestroy()) {
            delete g_Enemies[i];
            g_Enemies[i] = g_Enemies[--g_EnemyCount];
        }
    }

}

void Enemy_Draw()
{
    for (int i = 0; i < g_EnemyCount; i++) {
        g_Enemies[i]->Draw();
    }
}

void Enemy_Create(const DirectX::XMFLOAT3& position)
{
    g_Enemies[g_EnemyCount++] = new EnemyNormal(position);
}

int Enemy_GetEnemyCount()
{
    return g_EnemyCount;
}

Enemy* Enemy_GetEnemy(int index)
{
    return g_Enemies[index];
}
