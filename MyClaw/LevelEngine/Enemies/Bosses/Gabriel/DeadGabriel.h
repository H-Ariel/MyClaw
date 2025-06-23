#pragma once

#include "../../../Objects/OneTimeAnimation.h"


// Plays Gabriel's animation when he loses
class DeadGabriel : public OneTimeAnimation
{
public:
	DeadGabriel(D2D1_POINT_2F pos, const unordered_map<string, shared_ptr<UIAnimation>>& gabrielAnimations);

private:
	void changeAnimation();

	Timer _timer;
};
