/*========================================================================================


    Player Camera View [player_camera.h]								PYAE SONE THANT
                                                                        DATE:10/31/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef PLAYER_CAMERA_H
#define PLAYER_CAMERA_H

#include <DirectXMath.h>

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

void PlayerCamera_ToggleMode();
CameraMode PlayerCamera_GetMode();

#endif // PLAYER_CAMERA_H
