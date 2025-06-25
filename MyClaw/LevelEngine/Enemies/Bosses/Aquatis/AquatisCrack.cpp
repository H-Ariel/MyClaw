#include "AquatisCrack.h"
#include "Aquatis.h"
#include "../../../GlobalObjects.h"


AquatisCrack::AquatisCrack(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _lastDynamite(nullptr)
{
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet));
	setObjectRectangle();
}
void AquatisCrack::Logic(uint32_t elapsedTime)
{
	for (Projectile* p : GO::getActionPlaneProjectiles())
	{
		if (p != _lastDynamite && p->isClawDynamite() &&
			p->getDamage() && _objRc.intersects(p->GetRect()))
		{
			_lastDynamite = p;
			Aquatis::getInstance()->activateNextStalactite();
		}
	}
}
