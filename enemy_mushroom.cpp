#include "enemy_mushroom.h"
#include "texture.h"
#include "sprite_anim.h"

using namespace DirectX;

EnemyMushroom::EnemyMushroom(const XMFLOAT3& position)
    : EnemyHumanoid(position)
{
    // ---------------- TEXTURES ----------------
    int texLeftWalk = Texture_Load(L"sprites/Mushroom/mushroom_left_walk.png");
    int texRightWalk = Texture_Load(L"sprites/Mushroom/mushroom_right_walk.png");

    int texLeftIdle = Texture_Load(L"sprites/mushroom/mushroom_left_idle.png");
    int texRightIdle = Texture_Load(L"sprites/mushroom/mushroom_right_idle.png");

    int texHitLeft = Texture_Load(L"sprites/mushroom/mushroom_hit_left.png");
    int texHitRight = Texture_Load(L"sprites/mushroom/mushroom_hit_right.png");

    int texDeath = Texture_Load(L"sprites/mushroom/mushroom_death.png");

    int texLeftAttack = Texture_Load(L"sprites/mushroom/mushroom_left_attack.png");
    int texRightAttack = Texture_Load(L"sprites/mushroom/mushroom_right_attack.png");

    // ---------------- WALK ----------------
    m_AnimLeftPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texLeftWalk, 8, 8, 0.2f, { 80,64 }, { 0,0 }, true));    

    m_AnimRightPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texRightWalk, 8, 8, 0.2f, { 80,64 }, { 0,0 }, true));

    // ---------------- IDLE ----------------
    m_AnimLeftIdlePlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texLeftIdle, 7, 7, 0.3f, { 80,64 }, { 0,0 }, true));

    m_AnimRightIdlePlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texRightIdle, 7, 7, 0.3f, { 80,64 }, { 0,0 }, true));

    // ---------------- HIT ----------------
    m_AnimHitLeftPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texHitLeft, 5, 5, 0.2f, { 80,64 }, { 0,0 }, false));

    m_AnimHitRightPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texHitRight, 5, 5, 0.2f, { 80,64 }, { 0,0 }, false));

    // ---------------- DEATH ----------------
    m_AnimDeathPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texDeath, 15, 15, 0.3f, { 80,64 }, { 0,0 }, false));


    m_AnimLeftAttackPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texLeftAttack, 10, 10, 0.3f, { 80,64 }, { 0,0 }, false));

    m_AnimRightAttackPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texRightAttack, 10, 10, 0.3f, { 80,64 }, { 0,0 }, false));

    // ---------------- STATS ----------------
    m_Hp = 500;
    m_WalkSpeed = 1.0f;
    m_ChaseSpeed = 2.5f;
    m_DetectionRadius = 3.5f;
    m_VisualOffset.y = 1.0f;

    m_AnimLeftChasePlayId = m_AnimLeftPlayId;
    m_AnimRightChasePlayId = m_AnimRightPlayId;

    // ---------------- START STATE ----------------
    ChangeState(new StatePatrol(this));
}
