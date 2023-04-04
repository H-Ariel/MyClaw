#include "Laser.h"
#include "../AssetsManager.h"
#include "../Player.h"


Laser::Laser(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _timeCounter(0), _isActive(false),
	_damage(obj.damage ? obj.damage : 1), _swapTime(obj.counter > 0 ? obj.counter : 1500)
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 60, true);
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

int8_t Laser::getDamage()
{
	return _isActive ? _damage : 0;
}
