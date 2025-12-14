/*==============================================================================

   BillBoard header[billboard.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef BILLBOARD_H
#define BILLBOARD_H

#include <DirectXMath.h>

void Billboard_Initialize();
void Billboard_Finalize();

void Billboard_SetViewMatrix(const DirectX::XMFLOAT4X4& view);

// Add color parameter (with default white)
void Billboard_Draw(
    int texId,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT2& scale,
    const DirectX::XMUINT4& tex_cut,
    const DirectX::XMFLOAT2& pivot,
    const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

void Billboard_Draw(
    int texId,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT2& scale,
    const DirectX::XMFLOAT4& tex_cut,
    const DirectX::XMFLOAT2& pivot = { 0.0f, 0.0f },
    const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f }
);



#endif //BILLBOARD_H
	
