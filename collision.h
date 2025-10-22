/*========================================================================================


    Collision Header code [collision.h]										PYAE SONE THANT
                                                                        DATE:07/03/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#ifndef COLLISION_H
#define COLLISION_H

#include <d3d11.h>
#include <DirectXMath.h>

struct Circle
{
    DirectX::XMFLOAT2 center;
    float radius;   
};

struct Box
{
    DirectX::XMFLOAT2 center;
    float half_width;
    float half_height;
};

bool Collision_IsOverlapCircle(const Circle& a, const Circle& b);
bool Collision_IsOverlapBox(const Box& a, const Box& b);


void Collision_DebugInitialized(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Collision_DebugFinalize();
void Collision_DebugDraw(const Circle& circle,const DirectX::XMFLOAT4 color = {1.0f,1.0f,0.0f,1.0f});
void Collision_DebugDraw(const Box& box, const  DirectX::XMFLOAT4 color);

#endif // BULLET_H