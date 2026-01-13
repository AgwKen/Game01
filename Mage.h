#ifndef MAGE_H
#define MAGE_H

#include "enemy_humanoid.h"
#include <DirectXMath.h>

class Mage : public EnemyHumanoid
{
public:
    Mage(const DirectX::XMFLOAT3& position);
};

#endif