/*==============================================================================

  Shader Terrain Header [shader_terrain.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/24/11
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_TERRAIN_H
#define SHADER_TERRAIN_H

#include <d3d11.h>
#include <DirectXMath.h>

bool ShaderTerrain_Initialize();
void ShaderTerrain_Finalize();

void ShaderTerrain_SetWorldMatrix(const DirectX::XMMATRIX& matrix);
void ShaderTerrain_SetViewMatrix(const DirectX::XMMATRIX& matrix);
void ShaderTerrain_SetProjectionMatrix(const DirectX::XMMATRIX& matrix);

void ShaderTerrain_Begin();

#endif // SHADER_TERRAIN_H
