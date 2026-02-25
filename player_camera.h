/*========================================================================================


    Player Camera View [player_camera.h]								PYAE SONE THANT
                                                                        DATE:10/31/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef PLAYER_CAMERA_H
#define PLAYER_CAMERA_H

#include <DirectXMath.h>
#include "shader_billboard.h"

void PlayerCamera_Initialize();
void PlayerCamera_Finalize();
void PlayerCamera_Update(double elapsed_time);

const DirectX::XMFLOAT3& PlayerCamera_GetFront();
const DirectX::XMFLOAT3& PlayerCamera_GetPosition();

enum class CameraMode
{
    PLAYER_FOLLOW, // The default camera following the player
    DEBUG_FREE      // The debug camera with free movement
};
void PlayerCamera_BeginKickCinematic();
void PlayerCamera_EndKickCinematic();
void PlayerCamera_SnapCloseNow();
void PlayerCamera_ResetKickCinematic();
void PlayerCamera_ToggleMode();
CameraMode PlayerCamera_GetMode();


const  DirectX::XMFLOAT4X4& PlayerCamera_GetViewMatrix();
const  DirectX::XMFLOAT4X4& PlayerCamera_GetPerspectiveMatrix();

#endif // PLAYER_CAMERA_H
