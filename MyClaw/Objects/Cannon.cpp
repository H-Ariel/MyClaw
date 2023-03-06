#include "Cannon.h"
#include "Projectile.h"
#include "../Player.h"
#include "../PathManager.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"


Cannon::Cannon(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _timeCounter(0), _shootDirection(NotShoot), _ballOffset(0)
{
//	const string imagesPath = PathManager::getImageSetPath(obj.imageSet);
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

	if ((_shootDirection == ToLeft && position.x > _player->position.x ||
		_shootDirection == ToRight && position.x < _player->position.x)
		&& abs(position.x - _player->position.x) <= 512
		&& abs(position.y - _player->position.y) <= 256
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
		ActionPlane::addPlaneObject(DBG_NEW CannonBall(obj));
	}

	if (_ani == _firing && _ani->isFinishAnimation())
	{
		_ani = _home;
	}

	_ani->Logic(elapsedTime);
}


TowerCannon::TowerCannon(const WwdObject& obj, Player* player)
	: Cannon(obj, player)
{
	if (obj.logic == "TowerCannonLeft") _shootDirection = ToLeft;
	else if (obj.logic == "TowerCannonRight") _shootDirection = ToRight;
	else throw Exception(__FUNCTION__ ": not Cannon");

	_ballOffset = 8;
}
