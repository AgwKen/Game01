/*========================================================================================


    Mesh Header [Mesh.h]										        PYAE SONE THANT
                                                                        DATE:09/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef MESH_H
#define MESH_H

#include <d3d11.h>

void Mesh_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Mesh_Finalize();
void Mesh_Draw(int repeatX = 1, int repeatZ = 1, float heightOffset = 0.0f,
    float offsetX = 0.0f, float offsetZ = 0.0f);


#endif // Mesh_H