/*===============================================================================

  3D Shader field [shader_field.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/09/26
---------------------------------------------------------------------------------

=================================================================================*/
#ifndef SHADER_FIELD_H
#define	SHADER_FIELD_H

#include <d3d11.h>
#include <DirectXMath.h>

bool ShaderField_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void ShaderField_Finalize();
void ShaderField_SetWorldMatrix(const DirectX::XMMATRIX& matrix);
void ShaderField_SetViewMatrix(const DirectX::XMMATRIX& matrix);
void ShaderField_SetProjectionMatrix(const DirectX::XMMATRIX& matrix);

void ShaderField_Begin();

#endif // SHADER_FIELD_H

