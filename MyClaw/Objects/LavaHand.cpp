#include "LavaHand.h"
#include "../ActionPlane.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Assets-Managers/PathManager.h"
#include "../Objects/EnemyProjectile.h"


LavaHand::LavaHand(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _delay(obj.speed), _timeCounter(0), _throwed(false)
{
	_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationPath(obj.imageSet), PathManager::getImageSetPath(obj.imageSet));
	_ani->loopAni = false;
}

void LavaHand::Logic(uint32_t elapsedTime)
{
	if (_timeCounter > 0)
	{
		_timeCounter -= elapsedTime;
		return;
	}
	
	_ani->Logic(elapsedTime);

	if (_ani->getFrameNumber() == 10 && !_throwed)
	{
		ActionPlane::addPlaneObject(DBG_NEW LavahandProjectile(position, _isMirrored));
		_throwed = true;
	}
	if (_ani->isFinishAnimation())
	{
		_ani->reset();
		_ani->loopAni = false;
		_throwed = false;
		_timeCounter = _delay;
	}
}
