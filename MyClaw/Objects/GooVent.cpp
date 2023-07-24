#include "GooVent.h"
#include "../AssetsManager.h"


GooVent::GooVent(const WwdObject& obj, Player* player)
	: BaseDamageObject(obj, player, obj.damage ? obj.damage : 1)
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/GOOVENT.ANI");
	setObjectRectangle();
}

bool GooVent::isDamage() const
{
	size_t i = _ani->getFrameNumber();
	return (6 < i && i < 11);
}
