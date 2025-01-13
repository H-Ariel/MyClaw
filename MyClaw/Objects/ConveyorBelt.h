#pragma once

#include "BasePlaneObject.h"


class ConveyorBelt : public BaseStaticPlaneObject
{
public:
	ConveyorBelt(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	
	void orderAnimation(map<int, map<int, int>>& belts);

private:
	float speed;
	const bool _canMoveCC;
};
