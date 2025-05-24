#include "DeadGabriel.h"


DeadGabriel::DeadGabriel(D2D1_POINT_2F pos, shared_ptr<UIAnimation> ani, int firstPartDuration)
	: OneTimeAnimation(pos, ani, false), _delay(firstPartDuration), _moved(false)
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
