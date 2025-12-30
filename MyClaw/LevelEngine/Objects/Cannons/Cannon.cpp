#include "Cannon.h"
#include "../Projectiles/CannonBall.h"
#include "../../GlobalObjects.h"


Cannon::Cannon(const WwdObject& obj)
	: BaseStaticPlaneObject(obj, true), _timeCounter(0), _shootDirection(NotShoot), _ballOffset(0)
{
	const string anisPath = PathManager::getAnimationSetPath(obj.imageSet);

	_home = AssetsManager::loadAnimation(anisPath + "/HOME.ANI", obj.imageSet);
	_home->updateFrames = false;
	_home->position = position;
	_firing = AssetsManager::loadCopyAnimation(anisPath + "/FIRING.ANI", obj.imageSet);
	_firing->position = position;
	
	_ani = _home;

	setObjectRectangle();
}

void Cannon::Logic(uint32_t elapsedTime)
{
	_timeCounter += elapsedTime;

	if ((_shootDirection == ToLeft && position.x > GO::playerPosition().x ||
		_shootDirection == ToRight && position.x < GO::playerPosition().x)
		&& abs(position.x - GO::playerPosition().x) <= 512
		&& abs(position.y - GO::playerPosition().y) <= 256
		&& _ani != _firing && _timeCounter >= 2000) // shoot new bullet each 2 seconds
	{
		_ani = _firing;
		_ani->reset();
		_timeCounter = 0;

		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y + _ballOffset;
		obj.speedX = _shootDirection == ToRight ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
		obj.damage = 15;
		GO::addObjectToActionPlane(DBG_NEW CannonBall(obj));
	}

	if (_ani == _firing && _ani->isFinishAnimation())
	{
		_ani = _home;
	}

	_ani->Logic(elapsedTime);
}
