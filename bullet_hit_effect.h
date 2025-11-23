/*==============================================================================

   Bullet effect after hit header[bullet_hit_effect.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef BULLET_HIT_EFFECT_H
#define BULLET_HIT_EFFECT_H

#include <DirectXMath.h>

void BulletHitEffect_Initialize();
void BulletHitEffect_Finalize(void);
void BulletHitEffect_Update();
void BulletHitEffect_Draw();

void BulletHitEffect_Create(const DirectX::XMFLOAT3& position);

#endif //BULLET_HIT_EFFECT_H

