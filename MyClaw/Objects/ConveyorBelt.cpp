#include "ConveyorBelt.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"


ConveyorBelt::ConveyorBelt(const WwdObject& obj, WwdRect rect)
	: BaseStaticPlaneObject(obj), speed(obj.speed / 1000.f)
{
	// TODO: better animationss (as sequence)
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 100, obj.speed > 0);

	myMemCpy(_objRc, Rectangle2D(
		obj.x + rect.left - 32, obj.y + rect.top - 32,
		obj.x + rect.right - 32, obj.y + rect.bottom - 32
	));
}

void ConveyorBelt::Logic(uint32_t elapsedTime)
{
	// TODO: CC move faster in tiles limits and we need to fix it
	if (tryCatchPlayer())
	{
		player->position.x += speed * elapsedTime;
	}
}
