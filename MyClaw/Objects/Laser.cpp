#include "Laser.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"


Laser::Laser(const WwdObject& obj)
	: BaseDamageObject(obj, obj.damage ? obj.damage : 1), _timeCounter(0),
	_isActive(false), _swapTime(obj.counter > 0 ? obj.counter : 1500)
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 50, false);
	setObjectRectangle();
}

void Laser::Logic(uint32_t elapsedTime)
{
	_timeCounter += elapsedTime;
	if (_timeCounter >= _swapTime)
	{
		_timeCounter = 0;
		_isActive = !_isActive;

		if (_isActive)
		{
			_ani->updateFrames = true;
		}
		else
		{
			_ani->updateFrames = false;
			_ani->reset();
		}
	}

	if (_isActive)
	{
		_ani->Logic(elapsedTime);
	}
}

bool Laser::isDamage() const
{
	return _isActive;
}
