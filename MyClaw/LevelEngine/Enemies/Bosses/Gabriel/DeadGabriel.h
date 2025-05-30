#pragma once

#include "../../../Objects/OneTimeAnimation.h"


// Plays the animation of Gabriel when he lose
class DeadGabriel : public OneTimeAnimation
{
public:
	DeadGabriel(D2D1_POINT_2F pos, const map<string, shared_ptr<UIAnimation>>& gabrielAnimations);
	void Logic(uint32_t elapsedTime) override;

private:
	int _delay;
	bool _moved;
};
