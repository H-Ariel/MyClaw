#include "Cannon.h"
#include "EnemyProjectile.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../ActionPlane.h"


Cannon::Cannon(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _timeCounter(0), _shootDirection(NotShoot), _ballOffset(0)
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

	if ((_shootDirection == ToLeft && position.x > player->position.x ||
		_shootDirection == ToRight && position.x < player->position.x)
		&& abs(position.x - player->position.x) <= 512
		&& abs(position.y - player->position.y) <= 256
		&& _ani != _firing && _timeCounter >= 2000) // shoot new bullet each 2 seconds
	{
		_ani = _firing;
		_ani->reset();
		_timeCounter = 0;

		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y + _ballOffset;
		obj.z = ZCoord;
		obj.speedX = _shootDirection == ToRight ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
		obj.damage = 15;
		ActionPlane::addPlaneObject(DBG_NEW CannonBall(obj));
	}

	if (_ani == _firing && _ani->isFinishAnimation())
	{
		_ani = _home;
	}

	_ani->Logic(elapsedTime);
}


TowerCannon::TowerCannon(const WwdObject& obj)
	: Cannon(obj)
{
	if (endsWith(obj.logic, "Left")) _shootDirection = ToLeft;
	else if (endsWith(obj.logic, "Right")) _shootDirection = ToRight;
	else throw Exception('"' + obj.logic + "\" is not a tower-cannon");
	_ballOffset = 8;
}


SkullCannon::SkullCannon(const WwdObject& obj)
	: Cannon(obj)
{
	if (_isMirrored) _shootDirection = ToLeft;
	else _shootDirection = ToRight;
	_ballOffset = 8;
}
