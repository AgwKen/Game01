/*========================================================================================


  SkyDome Header [sky.h]										        PYAE SONE THANT
                                                                        DATE:11/21/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef SKY_H
#define SKY_H

#include <DirectXMath.h>

void Sky_Initialize();
void Sky_Finalize();
void Sky_SetPosition(const DirectX::XMFLOAT3& position);
void Sky_Draw();

#endif // SKY_H

