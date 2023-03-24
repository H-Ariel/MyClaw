#pragma once

#include "../BasePlaneObject.h"


class TProjectilesShooter : public BaseStaticPlaneObject
{
public:
	TProjectilesShooter(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	const D2D1_POINT_2F _speed, _offset;
	const int32_t _maxRestTime, _damage;
	int32_t _restTime;
};
