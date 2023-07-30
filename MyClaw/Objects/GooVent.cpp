#include "GooVent.h"
#include "../Assets-Managers/AssetsManager.h"


GooVent::GooVent(const WwdObject& obj)
	: BaseDamageObject(obj, obj.damage ? obj.damage : 1)
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/GOOVENT.ANI");
	setObjectRectangle();
}

bool GooVent::isDamage() const
{
	size_t i = _ani->getFrameNumber();
	return (6 < i && i < 11);
}
