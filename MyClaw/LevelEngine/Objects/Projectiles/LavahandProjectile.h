#pragma once

#include "EnemyProjectile.h"


class LavahandProjectile : public EnemyProjectile
{
public:
	LavahandProjectile(D2D1_POINT_2F initialPosition, bool mirrored);
	~LavahandProjectile();
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
};
