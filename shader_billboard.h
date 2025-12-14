/*==============================================================================

  Shader BillBoard Header [shader_billboard.h]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/11/14
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_BILLBOARD_H
#define SHADER_BILLBOARD_H

#include <d3d11.h>
#include <DirectXMath.h>

// ---------------------------------------------------------
// Unified UVParameter (used by fog + billboards)
// Must match HLSL cbuffer register(b3)
// ---------------------------------------------------------
struct UVParameter
{
    DirectX::XMFLOAT2 scale;        // sprite scaling
    DirectX::XMFLOAT2 translation;  // sprite UV offset for atlas

    DirectX::XMFLOAT2 uvOffset;     // fog scrolling offset (NEW)
    float alpha;                    // fog transparency (NEW)
    float pad;                      // alignment padding (MUST HAVE)
};

// ---------------------------------------------------------

bool ShaderBillBoard_Initialize();
void ShaderBillBoard_Finalize();

void ShaderBillBoard_SetWorldMatrix(const DirectX::XMMATRIX& matrix);
void ShaderBillBoard_SetColor(const DirectX::XMFLOAT4& color);

void ShaderBillBoard_SetUVParameter(const UVParameter& parameter);

void ShaderBillBoard_Begin();

#endif // SHADER_BILLBOARD_H
