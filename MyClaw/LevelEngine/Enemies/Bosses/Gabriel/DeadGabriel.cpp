#include "DeadGabriel.h"


DeadGabriel::DeadGabriel(D2D1_POINT_2F pos, const unordered_map<string, shared_ptr<UIAnimation>>& gabrielAnimations)
	: OneTimeAnimation(pos, make_shared<UIAnimation>(
		gabrielAnimations.at("KILLFALL1")->getFramesList() +
		gabrielAnimations.at("KILLFALL2")->getFramesList() +
		gabrielAnimations.at("KILLFALL3")->getFramesList() +
		gabrielAnimations.at("KILLFALL4")->getFramesList()
	), false),
	_timer((int)gabrielAnimations.at("KILLFALL1")->getTotalDuration(),
		bind(&DeadGabriel::changeAnimation, this))
{
	drawZ = DefaultZCoord::Characters + 1;
	addTimer(&_timer);
}

void DeadGabriel::changeAnimation()
{
	// TODO: add flip in air before stay in this position
	_ani->position = { 42992, 5240 };
	_ani->mirrored = true;
}
