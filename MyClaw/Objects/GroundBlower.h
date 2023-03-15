#pragma once

#include "../BasePlaneObject.h"


class GroundBlower : public BaseStaticPlaneObject
{
public:
	GroundBlower(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	shared_ptr<Animation> _blowhole;
	const float _force; // the force calculated according to formula: V^2 = V0^2 + 2a * Y
};
