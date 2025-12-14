/*==============================================================================

  Circle Shadow[circle_shadow.h]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/12/12
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef CIRCLE_SHADOW_H
#define CIRCLE_SHADOW_H

#include <d3d11.h>
#include <DirectXMath.h>

void CircleShadow_Initialize();
void CircleShadow_Finalize();
void CircleShadow_Draw(const DirectX :: XMFLOAT3 & position);

#endif //FOG_H


