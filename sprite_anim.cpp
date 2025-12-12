/*==============================================================================

    スプライトアニメーション [sprite_anime.cpp]

    Author : PYAE SONE THANT
    Date   : 2025/06/17

==============================================================================*/

#include "sprite_anim.h"
#include "sprite.h"
#include "texture.h"
#include "billboard.h"

#include <DirectXMath.h>
using namespace DirectX;

static constexpr int ANIM_PATTERN_MAX = 128;
static constexpr int ANIM_PLAY_MAX = 256;


struct AnimPatternData 
{
    int m_TextureId{ -1 };                   // テクスチャID
    int m_PatternMax = 0;                    // パターン数
    int m_HPatternMax = 0;                   // 横のパターン最大数
    XMUINT2 m_StartPosition = { 0, 0 };      // アニメーションのスタート座標
    XMUINT2 m_PatternSize = { 0, 0 };        // 1パターンの幅と高さ
    double m_seconds_per_pattern = 0.1;      // 各パターンの表示時間
    bool m_IsLooped = true;                  // ループするか
};

struct AnimPlayData 
{
    int m_PatternId = -1;                    // 登録されたアニメーションパターンID
    int m_PatternNum = 0;                    // 現在のパターン番号
    double m_AccumulatedTime = 0.0;          // 累積時間
    bool m_IsStopped = false;
};

static AnimPatternData g_AnimPattern[ANIM_PATTERN_MAX];
static AnimPlayData g_AnimPlay[ANIM_PLAY_MAX];

void SpriteAnim_Initialize() {
    for (AnimPatternData& data : g_AnimPattern) {
        data.m_TextureId = -1;
    }

    for (AnimPlayData& data : g_AnimPlay) {
        data.m_PatternId = -1;
        data.m_IsStopped = false;
    }
}

void SpriteAnim_Finalize()
{

}

void SpriteAnim_Update(double elapsed_time) {
    for (int i = 0; i < ANIM_PLAY_MAX; ++i) {
        if (g_AnimPlay[i].m_PatternId < 0) continue;
        if (g_AnimPlay[i].m_IsStopped) continue;

        AnimPatternData* pAnimPatternData = &g_AnimPattern[g_AnimPlay[i].m_PatternId];

        g_AnimPlay[i].m_AccumulatedTime += elapsed_time;

        if (g_AnimPlay[i].m_AccumulatedTime >= pAnimPatternData->m_seconds_per_pattern) {
            g_AnimPlay[i].m_PatternNum++;

            if (g_AnimPlay[i].m_PatternNum >= pAnimPatternData->m_PatternMax) {
                if (pAnimPatternData->m_IsLooped) {
                    g_AnimPlay[i].m_PatternNum = 0;
                }
                else {
                    g_AnimPlay[i].m_PatternNum = pAnimPatternData->m_PatternMax - 1;
                    g_AnimPlay[i].m_IsStopped = true;
                }
            }

            g_AnimPlay[i].m_AccumulatedTime -= pAnimPatternData->m_seconds_per_pattern;
        }
    }
}

void SpriteAnim_UpdatePlayer(int playid, double elapsed_time)
{
    if (playid < 0 || playid >= ANIM_PLAY_MAX) return;

    AnimPlayData& player = g_AnimPlay[playid];
    if (player.m_PatternId < 0 || player.m_IsStopped) return;

    AnimPatternData* pPattern = &g_AnimPattern[player.m_PatternId];

    player.m_AccumulatedTime += elapsed_time;

    if (player.m_AccumulatedTime >= pPattern->m_seconds_per_pattern)
    {
        player.m_PatternNum++;

        if (player.m_PatternNum >= pPattern->m_PatternMax)
        {
            if (pPattern->m_IsLooped)
                player.m_PatternNum = 0;
            else
            {
                player.m_PatternNum = pPattern->m_PatternMax - 1;
                player.m_IsStopped = true;
            }
        }

        player.m_AccumulatedTime -= pPattern->m_seconds_per_pattern;
    }
}


void SpriteAnim_Draw(int playid, float dx, float dy, float dw, float dh) 
{
    int patternId = g_AnimPlay[playid].m_PatternId;
    if (patternId < 0) return;

    const AnimPatternData* pPattern = &g_AnimPattern[patternId];
    int patternNum = g_AnimPlay[playid].m_PatternNum;

    int srcX = pPattern->m_StartPosition.x + (pPattern->m_PatternSize.x * (patternNum % pPattern->m_HPatternMax));
    int srcY = pPattern->m_StartPosition.y + (pPattern->m_PatternSize.y * (patternNum / pPattern->m_HPatternMax));

    int drawSrcX = srcX;
    int drawSrcY = srcY;
    int drawSrcW = pPattern->m_PatternSize.x;
    int drawSrcH = pPattern->m_PatternSize.y;

    Sprite_Draw(pPattern->m_TextureId, dx, dy, dw, dh,
        drawSrcX, drawSrcY, drawSrcW, drawSrcH);
}

void BillboardAnim_Draw(int playid, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& scale, const DirectX::XMFLOAT2& pivot)
{
    int anim_pattern_id = g_AnimPlay[playid].m_PatternId;
    if (anim_pattern_id < 0) return; // Add check for valid pattern ID

    const AnimPatternData* pAnimPatternData = &g_AnimPattern[anim_pattern_id];
    DirectX::XMUINT4 tex_cut = {
        pAnimPatternData->m_StartPosition.x
        + pAnimPatternData->m_PatternSize.x
        * (g_AnimPlay[playid].m_PatternNum % pAnimPatternData->m_HPatternMax),
        pAnimPatternData->m_StartPosition.y
        + pAnimPatternData->m_PatternSize.y
        * (g_AnimPlay[playid].m_PatternNum / pAnimPatternData->m_HPatternMax),
        pAnimPatternData->m_PatternSize.x,
        pAnimPatternData->m_PatternSize.y
    };

    Billboard_Draw(pAnimPatternData->m_TextureId,
        position, scale,
        tex_cut, // Pass the explicitly typed XMUINT4
        pivot
    );

}

int SpriteAnim_RegisterPattern(int texId, int pattern_max, int h_pattern_max, double seconds_per_pattern,
    const XMUINT2& pattern_size, const XMUINT2& start_position, bool is_looped) {
    for (int i = 0; i < ANIM_PATTERN_MAX; ++i) {
        if (g_AnimPattern[i].m_TextureId >= 0) continue;

        g_AnimPattern[i].m_TextureId = texId;
        g_AnimPattern[i].m_PatternMax = pattern_max;
        g_AnimPattern[i].m_HPatternMax = h_pattern_max;
        g_AnimPattern[i].m_seconds_per_pattern = seconds_per_pattern;
        g_AnimPattern[i].m_PatternSize = pattern_size;
        g_AnimPattern[i].m_StartPosition = start_position;
        g_AnimPattern[i].m_IsLooped = is_looped;

        return i;
    }
    return -1;
}

int SpriteAnim_CreatePlayer(int anim_pattern_id) {
    for (int i = 0; i < ANIM_PLAY_MAX; ++i) {
        if (g_AnimPlay[i].m_PatternId >= 0) continue;

        g_AnimPlay[i].m_PatternId = anim_pattern_id;
        g_AnimPlay[i].m_AccumulatedTime = 0.0;
        g_AnimPlay[i].m_PatternNum = 0;

        g_AnimPlay[i].m_IsStopped = false;

        return i;
    }
    return -1;
}

bool SpriteAnim_IsStopped(int index)
{
    return g_AnimPlay[index].m_IsStopped;
}

void SpriteAnim_DestroyPlayer(int index)
{
    g_AnimPlay[index].m_PatternId = -1;
}

void SpriteAnim_SetFrame(int playid, int frameIndex)
{
    if (playid < 0 || playid >= ANIM_PLAY_MAX)
        return;

    AnimPlayData& player = g_AnimPlay[playid];

    if (player.m_PatternId < 0)
        return;

    AnimPatternData& pattern = g_AnimPattern[player.m_PatternId];

    // Clamp frameIndex to valid range
    if (frameIndex < 0)
        frameIndex = 0;
    else if (frameIndex >= pattern.m_PatternMax)
        frameIndex = pattern.m_PatternMax - 1;

    // Set the current frame
    player.m_PatternNum = frameIndex;

    // Reset accumulated time so it won't advance automatically immediately
    player.m_AccumulatedTime = 0.0;

    player.m_IsStopped = true;
}

void SpriteAnim_Resume(int playid)
{
    if (playid < 0 || playid >= ANIM_PLAY_MAX)
        return;
    g_AnimPlay[playid].m_IsStopped = false;
}

void SpriteAnim_Pause(int playid)
{
    if (playid < 0 || playid >= ANIM_PLAY_MAX) return;
    g_AnimPlay[playid].m_IsStopped = true;
}
