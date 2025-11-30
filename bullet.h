/*========================================================================================


  Bullet Header [bullet.h]										        PYAE SONE THANT
                                                                        DATE:12/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/

#ifndef BULLET_H
#define BULLET_H

#include <DirectXMath.h>
using namespace DirectX;
#include "collision.h"

void Bullet_Initialize();
void Bullet_Finalize();
void Bullet_Update(double elapsed_time);
void Bullet_Draw();

void Bullet_Create(const XMFLOAT3& position, const XMFLOAT3& velocity);
void Bullet_Destroy(int index);

int Bullet_GetBulletsCount();

AABB Bullet_GetAABB(int index);
Sphere Bullet_GetSphere(int index);
const DirectX :: XMFLOAT3& Bullet_GetPosition(int index);


#endif // BULLET_H
