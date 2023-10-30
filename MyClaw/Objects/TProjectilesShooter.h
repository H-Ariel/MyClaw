#pragma once

#include "Projectile.h"


class TProjectilesShooter : public BaseStaticPlaneObject
{
public:
	TProjectilesShooter(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _projectileAni;
	const D2D1_POINT_2F speed, _offset;
	const int32_t _maxRestTime, _damage;
	int32_t _restTime;
};
