/*==============================================================================

  Shader depth [shader3d.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/09/09
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_DEPTH_H
#define	SHADER_DEPTH_H

#include <d3d11.h>
#include <DirectXMath.h>

bool ShaderDepth_Initialize();
void ShaderDepth_Finalize();

void ShaderDepth_SetWorldMatrix(const DirectX::XMMATRIX& matrix);
void ShaderDepth_Begin();

#endif // SHADER_DEPTH_H
