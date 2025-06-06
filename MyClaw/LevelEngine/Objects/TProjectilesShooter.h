#pragma once

#include "Projectiles/Projectile.h"


class TProjectilesShooter : public BaseStaticPlaneObject
{
public:
	TProjectilesShooter(const WwdObject& obj, int levelNumber);

	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<UIAnimation> _projectileAni;
	const D2D1_POINT_2F _projSpeed, _offset;
	const int _maxRestTime, _damage;
	int _shootIndex;
	int _restTime;
	bool _projIsOut;
};
