#ifndef ENEMY_MUSHROOM_H
#define ENEMY_MUSHROOM_H

#include "enemy_humanoid.h"
#include <DirectXMath.h>

class EnemyMushroom : public EnemyHumanoid
{
public:
    EnemyMushroom(const DirectX::XMFLOAT3& position);
};

#endif
