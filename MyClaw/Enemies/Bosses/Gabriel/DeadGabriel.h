#pragma once

#include "../../../Objects/BasePlaneObject.h"


class DeadGabriel : public OneTimeAnimation
{
public:
	DeadGabriel(D2D1_POINT_2F pos, shared_ptr<UIAnimation> ani, int firstPartDuration);
	void Logic(uint32_t elapsedTime) override;

private:
	int _delay;
	bool _moved;
};