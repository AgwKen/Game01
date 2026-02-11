/*========================================================================================


    Sampler Header [sampler.h]										        PYAE SONE THANT
                                                                           DATE:09/18/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef SAMPLER_H
#define SAMPLER_H

#include <d3d11.h>

void Sampler_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Sampler_Finalize();

// Existing functions
void Sampler_SetFilterPoint();
void Sampler_SetFilterLinear();
void Sampler_SetFilterAnisotropic();

// NEW: Retrieval functions for Shadow Shader
ID3D11SamplerState* Sampler_GetClamp();
ID3D11SamplerState* Sampler_GetWrap();
void Sampler_SetClamp();


#endif // SAMPLER_H