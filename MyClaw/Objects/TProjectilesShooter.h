#pragma once

#include "Projectile.h"


class TProjectilesShooter : public BaseStaticPlaneObject
{
public:
	TProjectilesShooter(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	const D2D1_POINT_2F speed, _offset;
	const int32_t _maxRestTime, _damage;
	int32_t _restTime;
};


class TProjectile : public Projectile
{
public:
	TProjectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);
};
