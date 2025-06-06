#pragma once

#include "EnemyProjectile.h"


class CannonBall : public EnemyProjectile
{
public:
	CannonBall(const WwdObject& obj)
		: EnemyProjectile(obj, PathManager::getImageSetPath("LEVEL_CANNONBALL")) {}
};
