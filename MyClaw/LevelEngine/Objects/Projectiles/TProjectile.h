#pragma once

#include "EnemyProjectile.h"

class TProjectile : public EnemyProjectile
{
public:
	TProjectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
		: EnemyProjectile(ani, damage, speed, initialPosition) {}
};
