/*========================================================================================


   Light Settings [light.h]										        PYAE SONE THANT
                                                                        DATE:09/30/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef LIGHT_H
#define LIGHT_H

#include <d3d11.h>
#include <DirectXMath.h>

void Light_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Light_Finalize();
void Light_SetAmbientColor(const DirectX:: XMFLOAT4& color);
void Light_SetDirectionalWorld(const DirectX::XMFLOAT4& world_directional, const DirectX::XMFLOAT4& color);

#endif // LIGHT_H


