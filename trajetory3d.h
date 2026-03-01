/*========================================================================================

   trajetory 3D Header[trajetory3d.h]                                  PYAE SONE THANT
                                                                        DATE:21/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef TRAJETORY3D_H
#define TRAJETORY3D_H

#include <DirectXMath.h>

// Initialize / Finalize
void Trajetory3d_Initialize();
void Trajetory3d_Finalize();

// Update / Draw
void Trajetory3d_Update(double elapsed_time);
void Trajetory3d_Draw();

// Spawn one trail particle
void Trajectory3d_Create(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT4& color,
    float size,
    double lifeTime
);

#endif // TRAJETORY3D_H