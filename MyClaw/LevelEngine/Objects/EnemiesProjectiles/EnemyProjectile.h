#pragma once

#include "../Projectile.h"


class EnemyProjectile : public Projectile
{
public:
	EnemyProjectile(const WwdObject& obj, const string& projectileAniDir)
		: Projectile(obj, PathManager::getImageSetPath(projectileAniDir)) {}

	EnemyProjectile(const WwdObject& obj, const string& projectileAni, const string& imageSet)
		: Projectile(obj, projectileAni, imageSet) {}

protected:
	EnemyProjectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
		: Projectile(ani, damage, speed, initialPosition) {}
};
