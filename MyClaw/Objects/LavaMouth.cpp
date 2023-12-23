#include "LavaMouth.h"


LavaMouth::LavaMouth(const WwdObject& obj)
	: BaseDamageObject(obj, 10), _startTimeDelay(obj.speed)
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_LAVAMOUTH"), false, 50);
}

void LavaMouth::Logic(uint32_t elapsedTime)
{
	if (_startTimeDelay > 0)
	{
		_startTimeDelay -= elapsedTime;
		return;
	}

	_ani->Logic(elapsedTime);
	setObjectRectangle();
}

bool LavaMouth::isDamage() const
{
	return true;
}
