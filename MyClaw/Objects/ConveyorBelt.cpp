#include "ConveyorBelt.h"
#include "../Player.h"
#include "../AssetsManager.h"


ConveyorBelt::ConveyorBelt(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _speed(obj.speed / 1000.f),
	_canMoveCC(contains(obj.imageSet, "MIDDLE"))
{
	// TODO: better animationss (as sequence)
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 75, obj.speed > 0);
	setObjectRectangle();
}

void ConveyorBelt::Logic(uint32_t elapsedTime)
{
	if (_canMoveCC && CollisionDistances::isCollision(_player->GetRect(), _objRc))
	{
		_player->position.x += _speed * elapsedTime;
	}
}
