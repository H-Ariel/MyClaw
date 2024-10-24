#include "LavaMouth.h"


LavaMouth::LavaMouth(const WwdObject& obj)
	: BaseDamageObject(obj, 10)
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_LAVAMOUTH"), false, 50);
	Delay(obj.speed);
}

void LavaMouth::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);
	setObjectRectangle();
}

bool LavaMouth::isDamage() const
{
	return true;
}
