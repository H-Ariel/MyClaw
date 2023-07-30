#include "ConveyorBelt.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"


ConveyorBelt::ConveyorBelt(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), speed(obj.speed / 1000.f),
	_canMoveCC(contains(obj.imageSet, "MIDDLE"))
{
	// TODO: better animationss (as sequence)
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 75, obj.speed > 0);
	setObjectRectangle();
}

void ConveyorBelt::Logic(uint32_t elapsedTime)
{
	// TODO: CC move faster in tiles limits
	if (_canMoveCC && player->GetRect().intersects(_objRc))
	{
		player->position.x += speed * elapsedTime;
	}
}
