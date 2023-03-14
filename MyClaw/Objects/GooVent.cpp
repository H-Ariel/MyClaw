#include "GooVent.h"
#include "../AssetsManager.h"


GooVent::GooVent(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _damage(obj.damage ? obj.damage : 1)
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/GOOVENT.ANI");
	setObjectRectangle();
}

int8_t GooVent::getDamage() const
{
	size_t i = _ani->getFrameNumber();
	if (6 < i && i < 11)
		return _damage;
	return 0;
}
