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

struct AABB
{
	DirectX::XMFLOAT3 min;
	DirectX::XMFLOAT3 max;

    DirectX::XMFLOAT3 GetCenter() const {
        DirectX::XMFLOAT3 center;

        center.x = min.x + (max.x - min.x) * 0.5f;
        center.y = min.y + (max.y - min.y) * 0.5f;
        center.z = min.z + (max.z - min.z) * 0.5f;

        return center;
	}
};

struct Hit
{
	bool isHit;
	DirectX::XMFLOAT3 normal;
};


bool Collision_IsOverlapCircle(const Circle& a, const Circle& b);
bool Collision_IsOverlapBox(const Box& a, const Box& b);
bool Collision_IsOverlapAABB(const AABB& a, const AABB& b);

Hit Collision_IsHitAABB(const AABB& a, const AABB& b);

void Collision_DebugInitialized(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Collision_DebugFinalize();
void Collision_DebugDraw(const Circle& circle,const DirectX::XMFLOAT4 color = {1.0f,1.0f,0.0f,1.0f});
void Collision_DebugDraw(const Box& box, const  DirectX::XMFLOAT4 color);

#endif // BULLET_H