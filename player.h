/*===============================================================================

  player header[player.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/10/31
---------------------------------------------------------------------------------

=================================================================================*/
#ifndef PLAYER_H
#define	PLAYER_H
#include <DirectXMath.h>
using namespace DirectX;
#include "collision.h"

void Player_Initialize(const XMFLOAT3& position, const XMFLOAT3& front);
void Player_Finalize();
void Player_Update(double elapsed_time);
void Player_Draw();

const XMFLOAT3& Player_GetPosition();
const XMFLOAT3& Player_GetFront();
AABB Player_GetAABB();

AABB Player_ConvertPositionToAABB(const XMVECTOR& position);

#endif // PLAYER_H