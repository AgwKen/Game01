/*==============================================================================

   Sprite animation[sprite.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/06/17
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SPRITE_ANIM_H
#define	SPRITE_ANIM_H
#include <DirectXMath.h>

void SpriteAnim_Initialize();
void SpriteAnim_Finalize();

void SpriteAnim_Update(double elapsed_time);
void SpriteAnim_Draw(int playid,float dx, float dy, float dw, float dh);

int SpriteAnim_RegisterPattern(
    int texid,int pattern_max,int h_pattern_max,
	double seconds_per_pattern,
	const DirectX::XMUINT2& pattern_size,
	const DirectX::XMUINT2& start_position,bool is_looped = true);

int SpriteAnim_CreatePlayer(int anim_pattern_id);

bool SpriteAnim_IsStopped(int index);
void SpriteAnim_DestroyPlayer(int index);


void SpriteAnim_SetFrame(int playid, int frameIndex);
void SpriteAnim_Resume(int playid);



#endif //SPRITE_ANIM_H
