#pragma once

#include "../BasePlaneObject.h"


class SpringBoard : public BaseStaticPlaneObject
{
public:
	SpringBoard(const WwdObject& obj, int levelNumber);

	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _idle, _spring;
	const float _force; // the force calculated according to formula: V^2 = V0^2 + 2a * Y
};
