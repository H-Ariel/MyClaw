#pragma once

#include "Projectile.h"


class TProjectilesShooter : public BaseStaticPlaneObject
{
public:
	TProjectilesShooter(const WwdObject& obj, int levelNumber);

	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<UIAnimation> _projectileAni;
	const D2D1_POINT_2F _projSpeed, _offset;
	const int32_t _maxRestTime, _damage;
	int _shootIndex;
	int32_t _restTime;
	bool _projIsOut;
};
