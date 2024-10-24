#include "LavaHand.h"
#include "../ActionPlane.h"
#include "../Objects/EnemyProjectile.h"


LavaHand::LavaHand(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _delay(obj.speed), _throwed(false)
{
	_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationPath(obj.imageSet), PathManager::getImageSetPath(obj.imageSet));
	_ani->loopAni = false;
}

void LavaHand::Logic(uint32_t elapsedTime)
{	
	_ani->Logic(elapsedTime);

	if (_ani->getFrameNumber() == 10 && !_throwed)
	{
		actionPlane->addPlaneObject(DBG_NEW LavahandProjectile(position, _isMirrored));
		_throwed = true;
	}
	if (_ani->isFinishAnimation())
	{
		_ani->reset();
		_ani->loopAni = false;
		_throwed = false;
		Delay(_delay);
	}
}
