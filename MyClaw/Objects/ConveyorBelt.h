#pragma once

#include "../BasePlaneObject.h"


class ConveyorBelt : public BaseStaticPlaneObject
{
public:
	ConveyorBelt(const WwdObject& obj, const WwdRect& rect);
	void Logic(uint32_t elapsedTime) override;
	
	static void GlobalInit();

private:
	float speed;
	const bool _canMoveCC;

	static vector<ConveyorBelt*> pConveyorBelts;
};
