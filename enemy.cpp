/*========================================================================================
 ? ? Enemy code [Enemy.h]
 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?PYAE SONE THANT
 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? DATE:07/02/2025
//------------------------------------------------------------------------------------------
//=========================================================================================*/
#include "enemy.h"
#include "bullet.h"
#include <DirectXMath.h>
#include "texture.h"
#include "sprite.h"
#include "sprite_anim.h"
using namespace DirectX;
#include "collision.h"
#include "effect.h"
#include "score.h"
#include "runner.h"
#include "Darkness.h"

// The Enemy struct definition has been moved to enemy.h
struct EnemyType {
    int texId;
    int tx, ty, tw, th;
    XMFLOAT2 velocity;
    Circle collision;
    int hp;
    unsigned int score;
};

static Enemy g_Enemies[ENEMY_MAX]{};
static int g_EnemyTexId = -1;
static constexpr float ENEMY_WIDTH = 64.0f;
static constexpr float ENEMY_HEIGHT = 64.0f;
static EnemyType g_EnemyTypes[ENEMY_TYPE_MAX];

void Enemy_Initialize() {
    for (Enemy& e : g_Enemies) {
        e.isEnable = false;
    }
  //  g_EnemyTypes[ENEMY_TYPE_zombie].texId = Texture_Load(L"Texture/Monster.png");
    g_EnemyTypes[ENEMY_TYPE_zombie].velocity = { 150.0f, 0.0f };
    g_EnemyTypes[ENEMY_TYPE_zombie].hp = 5;
    g_EnemyTypes[ENEMY_TYPE_zombie].score = 500;
    // Configure ShadowHandLeft enemy
    g_EnemyTypes[ENEMY_TYPE_ShadowHandLeft].texId = Texture_Load(L"Texture/ShadowHandLeft.png");
    g_EnemyTypes[ENEMY_TYPE_ShadowHandLeft].velocity = { -150.0f, 0.0f };
    g_EnemyTypes[ENEMY_TYPE_ShadowHandLeft].hp = 5;
    g_EnemyTypes[ENEMY_TYPE_ShadowHandLeft].score = 500;
    g_EnemyTypes[ENEMY_TYPE_ShadowHandLeft].collision = { { 200.0f, 32.0f }, 32.0f };
    // Configure ShadowHandRight enemy
    g_EnemyTypes[ENEMY_TYPE_ShadowHandRight].texId = Texture_Load(L"Texture/ShadowHandRight.png");
    g_EnemyTypes[ENEMY_TYPE_ShadowHandRight].velocity = { 150.0f, 0.0f };
    g_EnemyTypes[ENEMY_TYPE_ShadowHandRight].hp = 5;
    g_EnemyTypes[ENEMY_TYPE_ShadowHandRight].score = 500;
    g_EnemyTypes[ENEMY_TYPE_ShadowHandRight].collision = { { 200.0f, 32.0f }, 32.0f };
}


void Enemy_Finalize() {

}

void Enemy_Update(double elapsed_time) {
    for (int i = 0; i < ENEMY_MAX; ++i) {
        Enemy& e = g_Enemies[i];
        if (!e.isEnable) continue;
        XMVECTOR position = XMLoadFloat2(&e.position);
        XMVECTOR velocity = XMLoadFloat2(&e.velocity);
        position += velocity * (float)elapsed_time;
        XMStoreFloat2(&e.position, position);
        XMStoreFloat2(&e.velocity, velocity);
        SpriteAnim_Update(elapsed_time);
        e.lifeTime += elapsed_time;
        if (e.position.x + ENEMY_WIDTH < 0.0f) {
            e.isEnable = false;
        }

        // Updated logic for ShadowHand enemies
        if (e.typeId == ENEMY_TYPE_ShadowHandLeft || e.typeId == ENEMY_TYPE_ShadowHandRight)
        {
            XMFLOAT2 runnerPosition = Runner_GetWorldPosition();
            bool isRunnerFacingRight = Runner_IsFacingRight();

            if (Darkness_IsFlashlightOn())
            {
                bool isRunnerFacingEnemy = false;
                if (isRunnerFacingRight && e.position.x > runnerPosition.x)
                {
                    isRunnerFacingEnemy = true;
                }
                else if (!isRunnerFacingRight && e.position.x < runnerPosition.x)
                {
                    isRunnerFacingEnemy = true;
                }

                if (isRunnerFacingEnemy)
                {
                    Enemy_Destroy(i);
                    Score_AddScore(g_EnemyTypes[e.typeId].score);
                    continue;
                }
            }
        }
    }
}

void Enemy_Draw() {
    for (int i = 0; i < ENEMY_MAX; i++) {
        Enemy& e = g_Enemies[i];
        if (!e.isEnable) continue;
        if (e.animPlayerId >= 0) {
            SpriteAnim_Draw(e.animPlayerId, e.position.x, e.position.y, 400.0f, 64.0f);
        }
        else {
            Sprite_Draw(
                g_EnemyTypes[e.typeId].texId,
                e.position.x,
                e.position.y + e.offsetY,
                g_EnemyTypes[e.typeId].tw,
                g_EnemyTypes[e.typeId].th,
                e.isDamage ? XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f } : XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f }
            );
        }
        e.isDamage = false;
    }
}

void Enemy_Create(int id, const DirectX::XMFLOAT2& position) {
    for (Enemy& e : g_Enemies) {
        if (!e.isEnable) {
            e.isEnable = true;
            e.typeId = id;
            e.offsetY = position.y;
            e.position = position;
            e.velocity = g_EnemyTypes[id].velocity;
            e.lifeTime = 0.0;
            e.hp = g_EnemyTypes[id].hp;
            e.collision = g_EnemyTypes[id].collision;
            e.isDamage = false;
            e.damageTimer = 0.0;
            if (id == ENEMY_TYPE_ShadowHandLeft || id == ENEMY_TYPE_ShadowHandRight) {
                int animPatternId = SpriteAnim_RegisterPattern(
                    g_EnemyTypes[id].texId,
                    8,
                    8,
                    0.4,
                    { 168, 48 },
                    { 0, 0 },
                    true
                );
                e.animPlayerId = SpriteAnim_CreatePlayer(animPatternId);
            }
            else {
                e.animPlayerId = -1;
            }
            break;
        }
    }
}

bool Enemy_IsEnable(int index) {
    return g_Enemies[index].isEnable;
}

Circle Enemy_GetCollision(int index) {
    int id = g_Enemies[index].typeId;
    float cx = g_Enemies[index].position.x + g_EnemyTypes[id].collision.center.x;
    float cy = g_Enemies[index].position.y + g_EnemyTypes[id].collision.center.y;
    return { { cx, cy }, g_EnemyTypes[id].collision.radius };
}

void Enemy_Destroy(int index) {
    g_Enemies[index].isEnable = false;
}

void Enemy_Damage(int index) {
    Enemy& e = g_Enemies[index];
    e.hp--;
    e.isDamage = true;
    if (e.hp <= 0) {
        Effect_Create(e.position);
        e.isEnable = false;
        Score_AddScore(g_EnemyTypes[e.typeId].score);
    }
}

int Enemy_GetCount() {
    return ENEMY_MAX;
}

Enemy* Enemy_GetEnemy(int index) {
    if (index >= 0 && index < ENEMY_MAX) {
        return &g_Enemies[index];
    }
    return nullptr;
}