/*========================================================================================


   3D cube [cube.h]										        		PYAE SONE THANT
                                                                        DATE:09/09/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef CUBE_H
#define CUBE_H

#include <d3d11.h>
#include <DirectXMath.h>

void CUBE_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void CUBE_Finalize(void);
void CUBE_Draw(float angle);


#endif // CUBE_H

