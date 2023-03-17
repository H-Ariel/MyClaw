#pragma once

#include "../BasePlaneObject.h"


class ConveyorBelt : public BaseStaticPlaneObject
{
public:
	ConveyorBelt(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

private:
	float _speed;
	bool _canMoveCC;
};
