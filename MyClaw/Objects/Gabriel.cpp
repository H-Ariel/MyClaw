#include "Gabriel.h"
#include "../Player.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"
#include "RedTailPirate.h"


// TODO: move into classes
static bool riseCannon = false;
static bool cannonIsUp = false;
static bool operateCannon = false;
static bool GabrielChangeSwitch = false;
static bool makeGabrielHurt = false;


#define ANIMATION_HITLOW		_animations["HITLOW"]
#define ANIMATION_SEND_PIRATES	_animations["STRIKE2"]
#define ANIMATION_ACTION_CANNON _animations["STRIKE3"]
#define ANIMATION_THROW_BOMBS	_animations["STRIKE7"]
#define ANIMATION_BLOCK			_animations["BLOCKED"]

#define OPERATE_CANNON_TIME	7000
#define THROW_BOMBS_TIME	4500
#define SEND_PIRATES_TIME	2000


// TODO: change the `KILLFALL4` to the original death animations' sequence
Gabriel::Gabriel(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "", "HITHIGH", "HITLOW", "KILLFALL4", "", "", "")
	, _throwBombsTime(THROW_BOMBS_TIME), _canThrowBombs(true)
	, _sendPiratesTime(SEND_PIRATES_TIME), _canSendPirates(true)
{
	_forward = false;
	_ani = getRandomInt(0, 1) == 0 ? _animations["IDLE1"] : _animations["IDLE2"];
	_health = 5; // we need hurt Gabriel only 5 times to win
}
void Gabriel::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;
	if (_ani == ANIMATION_BLOCK && !_ani->isFinishAnimation())
	{
		PostLogic(elapsedTime);
		return;
	}

	// throw bombs
	if (_canThrowBombs)
	{
		_throwBombsTime -= elapsedTime;
		if (_throwBombsTime <= 0)
		{
			WwdObject obj;
			obj.x = (int32_t)position.x;
			obj.y = (int32_t)position.y;
			obj.z = ZCoord;
			obj.damage = 5;
			obj.speedY = -800;

			int32_t speedXs[] = {
				getRandomInt(-600, -467),
				getRandomInt(-467, -334),
				getRandomInt(-334, -200)
			};

			for (int i = 0; i < 3; i++) // throw 3 bombs
			{
				obj.speedX = speedXs[i];
				ActionPlane::addPlaneObject(DBG_NEW GabrielBomb(obj));
			}

			_ani = ANIMATION_THROW_BOMBS;
			_ani->reset();
			_ani->loopAni = false;
			_forward = false;
			_canThrowBombs = false;
		}
	}

	// send pirates
	if (_canSendPirates)
	{
		_sendPiratesTime -= elapsedTime;
		if (_sendPiratesTime <= 0)
		{
			ActionPlane::addPlaneObject(DBG_NEW Gabriel_RedTailPirate(_player));
			_ani = ANIMATION_SEND_PIRATES;
			_ani->reset();
			_ani->loopAni = false;
			_forward = false;
			_canSendPirates = false;
		}
	}

	if (GabrielChangeSwitch)
	{
		_ani = ANIMATION_ACTION_CANNON;
		_ani->reset();
		_ani->loopAni = false;
		_forward = true;
		GabrielChangeSwitch = false;
		_throwBombsTime = THROW_BOMBS_TIME; // throw bombs after `THROW_BOMBS_TIME` ms
		_canThrowBombs = true;
		_sendPiratesTime = SEND_PIRATES_TIME; // send pirates after `SEND_PIRATES_TIME` ms
		_canSendPirates = true;
	}

	if (makeGabrielHurt)
	{
		_ani = ANIMATION_HITLOW;
		_ani->reset();
		_ani->loopAni = false;
		_forward = false;
		makeGabrielHurt = false;
		_health -= 1;
		removeObject = (_health <= 0);
	}

	if ((_ani != _animations.at("IDLE1") && _ani != _animations.at("IDLE2")) && _ani->isFinishAnimation())
	{
		_ani = getRandomInt(0, 1) == 0 ? _animations["IDLE1"] : _animations["IDLE2"];
		_forward = false;
	}

	PostLogic(elapsedTime);
}
pair<Rectangle2D, int8_t> Gabriel::GetAttackRect() { return {}; }
bool Gabriel::checkForHurts()
{
	// block all CC attacks

	int16_t health = _health; // save health value

	if (BaseEnemy::checkForHurts())
	{
		_ani = ANIMATION_BLOCK;
		_ani->reset();
		_ani->loopAni = false;
	}

	// make sure value did not changed
	_health = health;
	removeObject = false;

	return false;
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

	cannonIsUp = _ani == _rise;

	if (operateCannon)
	{
		// shoot cannon ball
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
		GabrielChangeSwitch = true;
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
	: BaseStaticPlaneObject(obj, player), _switchCannonTime(OPERATE_CANNON_TIME)
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/SWITCH.ANI", PathManager::getImageSetPath(obj.imageSet));
	_ani->position = position;
	_ani->updateFrames = false;
}
void CannonSwitch::Logic(uint32_t elapsedTime)
{
	_switchCannonTime -= elapsedTime;
	if (_switchCannonTime <= 0) // use cannon every OPERATE_CANNON_TIME milliseconds
	{
		_switchCannonTime = OPERATE_CANNON_TIME;
		operateCannon = true;
		_ani->reset();
		_ani->loopAni = false;
	}
	_ani->Logic(elapsedTime);
}


CannonButton::CannonButton(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player)
{
	_idle = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/IDLE.ANI", PathManager::getImageSetPath(obj.imageSet));
	_pressed = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");
	_ani = _idle;
	setObjectRectangle();
}
void CannonButton::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);

	if (_ani == _pressed && !cannonIsUp)
	{
		_ani = _idle;
	}
	else if (_player->GetRect().intersects(_objRc) || _player->GetAttackRect().first.intersects(_objRc))
	{
		_ani = _pressed;
		riseCannon = true;
	}
}


Gabriel_RedTailPirate::Gabriel_RedTailPirate(Player* player)
	: BaseDynamicPlaneObject({}, player), _isJumping(false)
{
	position = { 43479, 5020 };
	myMemCpy(ZCoord, 2000U);
	_speed.x = -0.3f;
	_ani = AssetsManager::loadCopyAnimation("LEVEL8/ANIS/REDTAILPIRATE/ADVANCE.ANI", "LEVEL8/IMAGES/REDTAILPIRATE");
}
void Gabriel_RedTailPirate::Logic(uint32_t elapsedTime)
{
	_speed.y += GRAVITY * elapsedTime;
	position.x += _speed.x * elapsedTime;
	position.y += _speed.y * elapsedTime;
	if (position.x < 43200 && !_isJumping)
	{
		_speed.y = -0.7f;
		_isJumping = true;
		position.y -= 1;
		_ani->updateFrames = false;
	}
	_ani->position = position;
	_ani->Logic(elapsedTime);
}
void Gabriel_RedTailPirate::stopFalling(float collisionSize)
{
	if (_isJumping)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.z = ZCoord;
		obj.minX = 42560;
		obj.maxX = 43200;
		obj.imageSet = "LEVEL_REDTAILPIRATE";
		ActionPlane::addPlaneObject(new RedTailPirate(obj, _player, true));

		removeObject = true;
	}
	else
	{
		BaseDynamicPlaneObject::stopFalling(collisionSize);
	}
}