#pragma once

#include "../BasePlaneObject.h"


class ConveyorBelt : public BaseStaticPlaneObject
{
public:
	ConveyorBelt(const WwdObject& obj, WwdRect rect);
	void Logic(uint32_t elapsedTime) override;
	
private:
	float speed;
};
