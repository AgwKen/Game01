/*========================================================================================

    Texture Management [texture.h]                                  PYAE SONE THANT
                                                                    DATE:09/04/2025

//------------------------------------------------------------------------------------------
//=========================================================================================*/
#ifndef TEXTURE_H
#define	TEXTURE_H
#include <d3d11.h>

void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Texture_Finalize(void);
int Texture_Load(const wchar_t* pFilename);
void Texture_AllRelease();
void Texture_Release(int texid);
void Texture_SetTexture(int texid, int slot = 0);
unsigned int Texture_Width(int texid);
unsigned int Texture_Height(int texid);

#endif // TEXTURE_H
