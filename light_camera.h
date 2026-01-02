/*========================================================================================
    Light Camera [light_camera.h]                                   PYAE SONE THANT

                                                                    DATE:26/11/2025

------------------------------------------------------------------------------------------
=========================================================================================*/
#ifndef LIGHT_CAMERA_H
#define LIGHT_CAMERA_H

#include <DirectXMath.h>

void LightCamera_Initialize(const DirectX::XMFLOAT3& world_directional, const DirectX::XMFLOAT3& position);
void LightCamera_Finalize();

void LightCamera_SetPosition(const DirectX::XMFLOAT3& position);
void LightCamera_SetFront(const DirectX::XMFLOAT3& front);

DirectX::XMMATRIX LightCamera_GetViewMatrix();
DirectX::XMMATRIX LightCamera_GetProjectionMatrix();

#endif // LIGHT_CAMERA_H