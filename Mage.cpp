#include "mage.h"
#include "texture.h"
#include "sprite_anim.h"

using namespace DirectX;

Mage::Mage(const XMFLOAT3& position)
    : EnemyHumanoid(position)
{
    // Adjust stats for a Mage
    m_Hp = 500;               // Lower HP than mushroom
    m_DetectionRadius = 10.0f; // Sees further
    m_VisualScale = { 2.0f, 2.0f };
 
    // ---------------- TEXTURES ----------------
    // Replace these paths with your actual Mage sprite paths
    int texLeftWalk = Texture_Load(L"sprites/Mage/mage_left_walk.png");
    int texRightWalk = Texture_Load(L"sprites/Mage/mage_right_walk.png");
    int texLeftIdle = Texture_Load(L"sprites/Mage/mage_left_idle.png");
    int texRightIdle = Texture_Load(L"sprites/Mage/mage_right_idle.png");
    int texHitLeft = Texture_Load(L"sprites/Mage/mage_hit_left.png");
    int texHitRight = Texture_Load(L"sprites/Mage/mage_hit_right.png");
    int texDeath = Texture_Load(L"sprites/Mage/mage_death.png");
    int texLeftShoot = Texture_Load(L"sprites/Mage/mage_left_attack.png");
    int texRightShoot = Texture_Load(L"sprites/Mage/mage_right_attack.png");

    // ---------------- WALK (8 frames example) ----------------
    m_AnimLeftPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texLeftWalk, 8, 8, 0.2f, { 140,140 }, { 0,0 }, true));

    m_AnimRightPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texRightWalk, 8, 8, 0.2f, { 140,140 }, { 0,0 }, true));

    m_AnimLeftChasePlayId = m_AnimLeftPlayId;
    m_AnimRightChasePlayId = m_AnimRightPlayId;

    // ---------------- IDLE (7 frames example) ----------------
    m_AnimLeftIdlePlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texLeftIdle, 10, 10, 0.3f, { 140,140 }, { 0,0 }, true));

    m_AnimRightIdlePlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texRightIdle, 10, 10, 0.3f, { 140,140}, { 0,0 }, true));

    // ---------------- HIT (5 frames example) ----------------
    m_AnimHitLeftPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texHitLeft, 3, 3, 0.2f, { 140,140 }, { 0,0 }, false));

    m_AnimHitRightPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texHitRight, 3, 3, 0.2f, { 140,140 }, { 0,0 }, false));

    // ---------------- DEATH (15 frames example) ----------------
    m_AnimDeathPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texDeath, 18, 18, 0.3f, { 140,140 }, { 0,0 }, false));

    // ---------------- SHOOT/ATTACK (10 frames example) ----------------
    // We assign the Mage's shooting animation to the Attack slots
    m_AnimLeftAttackPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texLeftShoot, 13, 13, 0.3f, { 140,140 }, { 0,0 }, false));

    m_AnimRightAttackPlayId = SpriteAnim_CreatePlayer(
        SpriteAnim_RegisterPattern(texRightShoot, 13, 13, 0.3f, { 140,140 }, { 0,0 }, false));

    // Start in Patrol state
    ChangeState(new StatePatrol(this));
}