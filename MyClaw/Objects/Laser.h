#pragma once

#include "../BasePlaneObject.h"


class Laser : public BaseStaticPlaneObject
{
public:
	Laser(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	int getDamage();

private:
	const int32_t _swapTime; // in milliseconds
	int32_t _timeCounter;
	int _damage;
	bool _isActive;
};
