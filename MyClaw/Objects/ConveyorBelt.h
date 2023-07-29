#pragma once

#include "../BasePlaneObject.h"


class ConveyorBelt : public BaseStaticPlaneObject
{
public:
	ConveyorBelt(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	float speed;
	bool _canMoveCC;
};
