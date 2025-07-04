#pragma once

#include "../BasePlaneObject.h"


class Cannon : public BaseStaticPlaneObject
{
public:
	Cannon(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

protected:
	enum ShootDirection : int8_t { NotShoot, ToLeft, ToRight };
	uint32_t _ballOffset;
	ShootDirection _shootDirection;

private:
	shared_ptr<UIAnimation> _home, _firing;
	uint32_t _timeCounter;


	friend class PunkRat;
};
