#include "DeadGabriel.h"


DeadGabriel::DeadGabriel(D2D1_POINT_2F pos, const unordered_map<string, shared_ptr<UIAnimation>>& gabrielAnimations)
	: OneTimeAnimation(pos, make_shared<UIAnimation>(
		gabrielAnimations.at("KILLFALL1")->getFramesList() +
		gabrielAnimations.at("KILLFALL2")->getFramesList() +
		gabrielAnimations.at("KILLFALL3")->getFramesList() +
		gabrielAnimations.at("KILLFALL4")->getFramesList()
	), false),
	_delay((int)gabrielAnimations.at("KILLFALL1")->getTotalDuration()), _moved(false)
{
	drawZ = DefaultZCoord::Characters + 1;
}
void DeadGabriel::Logic(uint32_t elapsedTime)
{
	if (!_moved)
	{
		_delay -= elapsedTime;
		if (_delay <= 0)
		{
			// TODO: add flip in air before stay in this position
			_ani->position = { 42992, 5240 };
			_ani->mirrored = true;
			_moved = true;
		}
	}

	_ani->Logic(elapsedTime);
}
