#include "Gabriel.h"
#include "../Player.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"


// TODO: move into classes
static bool riseCannon = false;
static bool operateCannon = false;
static bool makeGabrielHurt = false;


#define ANIMATION_IDLE _animations["IDLE1"]
#define ANIMATION_HITLOW _animations["HITLOW"]

#define SWITCH_CANNON_TIME 3000 // TODO: change to 5000


// TODO: continue write Gabriel class

// TODO: change the `KILLFALL4` to the original death animations' sequence

Gabriel::Gabriel(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "", "HITHIGH", "HITLOW", "KILLFALL4", "", "", "")
{
	_forward = false;
	_ani = ANIMATION_IDLE;
	_health = 5; // we need hurt Gabriel only 5 times to win
}

void Gabriel::Logic(uint32_t elapsedTime)
{
	PreLogic(elapsedTime);

	// TODO: write logic (throw bombs, send pirates)

	if (makeGabrielHurt)
	{
		_ani = ANIMATION_HITLOW;
		_ani->reset();
		_ani->loopAni = false;
		makeGabrielHurt = false;
		_health -= 1;
		removeObject = (_health <= 0);
	}

	if (_ani == ANIMATION_HITLOW && _ani->isFinishAnimation())
	{
		_ani = ANIMATION_IDLE;
	}

	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> Gabriel::GetAttackRect()
{
	return {};
}

void Gabriel::makeAttack()
{
}

bool Gabriel::checkForHurts()
{
	// TODO: block all CC attacks
	return BaseEnemy::checkForHurts();
}




GabrielCannon::GabrielCannon(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player)
{
	string aniSet = PathManager::getAnimationSetPath(obj.imageSet);
	string imageSet = PathManager::getImageSetPath(obj.imageSet);

	_home = AssetsManager::loadAnimation(aniSet + "/HOME.ANI", imageSet); // nop
	_rest = AssetsManager::loadAnimation(aniSet + "/REST.ANI", imageSet); // go down
	_rise = AssetsManager::loadAnimation(aniSet + "/RISE.ANI", imageSet); // go up
	_horzfire = AssetsManager::loadAnimation(aniSet + "/HORZFIRE.ANI", imageSet); // horizontal fire
	_vertfire = AssetsManager::loadAnimation(aniSet + "/VERTIFIRE.ANI", imageSet); // vertical fire

	_ani = _home;

	_ani->position = position;
}

void GabrielCannon::Logic(uint32_t elapsedTime)
{
	const shared_ptr<const Animation> prevAni = _ani;

	if (_ani->isFinishAnimation())
	{
		if (_ani == _vertfire)
		{
			_ani = _rest;
		}
		else if (_ani == _horzfire || _ani == _rest)
		{
			_ani = _home;
		}
	}


	if (riseCannon)
	{
		_ani = _rise;
		riseCannon = false;
	}

	if (operateCannon)
	{
		// send cannonball
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.z = ZCoord;
		obj.damage = 15;

		if (_ani == _rise)
		{
			_ani = _vertfire;
			obj.speedY = -DEFAULT_PROJECTILE_SPEED;
			makeGabrielHurt = true;
		}
		else
		{
			_ani = _horzfire;
			obj.speedX = -DEFAULT_PROJECTILE_SPEED;
			obj.y += 56;
		}

		ActionPlane::addPlaneObject(DBG_NEW CannonBall(obj));

		operateCannon = false;
	}
	

	if (prevAni != _ani)
	{
		_ani->reset();
		_ani->loopAni = false;
		_ani->position = position;
	}

	_ani->Logic(elapsedTime);
}





CannonSwitch::CannonSwitch(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _switchCannonTime(SWITCH_CANNON_TIME)
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/SWITCH.ANI", PathManager::getImageSetPath(obj.imageSet));
	_ani->position = position;
	_ani->updateFrames = false;
}

void CannonSwitch::Logic(uint32_t elapsedTime)
{
	if (_switchCannonTime > 0)
		_switchCannonTime -= elapsedTime;
	if (_switchCannonTime <= 0) // use cannon every SWITCH_CANNON_TIME milliseconds
	{
		_switchCannonTime = SWITCH_CANNON_TIME;
		operateCannon = true;
		_ani->reset();
		_ani->loopAni = false;
	}

	_ani->Logic(elapsedTime);
}





CannonButton::CannonButton(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _pressedTime(0)
{
	_idle = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/IDLE.ANI", PathManager::getImageSetPath(obj.imageSet));
	_pressed = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");

	_ani = _idle;

	setObjectRectangle();
}

void CannonButton::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);

	if (_ani == _pressed)
	{
		_pressedTime += elapsedTime;
		if (_pressedTime >= 1500)
		{
			_ani = _idle;
			_pressedTime = 0;
		}
	}
	else
	{
		if (_player->GetRect().intersects(_objRc) || _player->GetAttackRect().first.intersects(_objRc))
		{
			_ani = _pressed;
			riseCannon = true;
		}
	}
}
