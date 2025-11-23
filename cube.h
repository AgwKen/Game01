/*========================================================================================


   3D cube [cube.h]										        		PYAE SONE THANT
                                                                        DATE:09/09/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef CUBE_H
#define CUBE_H

#include <d3d11.h>
#include <DirectXMath.h>
#include "collision.h"

void CUBE_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void CUBE_Finalize(void);
void CUBE_Draw(int texId,const DirectX::XMMATRIX& mtxWorld);

AABB Cube_GetAABB(const DirectX::XMFLOAT3& position);

#endif // CUBE_H

