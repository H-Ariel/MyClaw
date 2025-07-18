#include "Gabriel.h"
#include "GabrielRedTailPirate.h"
#include "GabrielBomb.h"
#include "DeadGabriel.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/Projectiles/CannonBall.h"


// TODO something better than all of this `static`...
static Gabriel* _Gabriel = nullptr;
static GabrielCannon* _GabrielCannon = nullptr;


#define ANIMATION_HITLOW		_animations["HITLOW"]
#define ANIMATION_SEND_PIRATES	_animations["STRIKE2"]
#define ANIMATION_ACTION_CANNON _animations["STRIKE3"]
#define ANIMATION_THROW_BOMBS	_animations["STRIKE7"]
#define ANIMATION_BLOCK			_animations["BLOCKED"]

constexpr int OPERATE_CANNON_TIME = 7000;
constexpr int THROW_BOMBS_TIME = 4500;
constexpr int SEND_PIRATES_TIME = 2000;



Gabriel::Gabriel(const WwdObject& obj)
	: BaseBoss(obj)
	, _throwBombsTime(THROW_BOMBS_TIME), _canThrowBombs(true)
	, _sendPiratesTime(SEND_PIRATES_TIME), _canSendPirates(true)
{
	_Gabriel = this;

	speed.x = 0;
	_isMirrored = true;
	_ani = _animations["IDLE"];
	_health = 5; // we need hurt Gabriel only 5 times to win
}
Gabriel::~Gabriel()
{
	_Gabriel = nullptr;
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW DeadGabriel(position,_animations));
		// TODO show gem only after DeadGabriel animation finished
	}
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
			obj.damage = 5;
			obj.speedY = -800;

			int speedXs[] = {
				getRandomInt(-600, -467),
				getRandomInt(-467, -334),
				getRandomInt(-334, -200)
			};

			for (int i = 0; i < 3; i++) // throw 3 bombs
			{
				obj.speedX = speedXs[i];
				GO::addObjectToActionPlane(DBG_NEW GabrielBomb(obj));
			}

			_ani = ANIMATION_THROW_BOMBS;
			_ani->reset();
			_ani->loopAni = false;
			_isMirrored = true;
			_canThrowBombs = false;
		}
	}

	// send pirates
	if (_canSendPirates)
	{
		_sendPiratesTime -= elapsedTime;
		if (_sendPiratesTime <= 0)
		{
			GO::addObjectToActionPlane(DBG_NEW GabrielRedTailPirate());
			_ani = ANIMATION_SEND_PIRATES;
			_ani->reset();
			_ani->loopAni = false;
			_isMirrored = true;
			_canSendPirates = false;
		}
	}

	if (_ani != _animations["IDLE"] && _ani->isFinishAnimation())
	{
		_ani = _animations["IDLE"];
		_isMirrored = true;
	}

	PostLogic(elapsedTime);
}
pair<Rectangle2D, int> Gabriel::GetAttackRect() { return {}; }
bool Gabriel::checkForHurts()
{
	// Gabriel blocks all CC attacks
	// cache fields to make sure BaseEnemy::checkForHurts does not change fields
	int healthCache = _health;
	bool removeObjectCache = removeObject;

	if (BaseEnemy::checkForHurts())
	{
		_ani = ANIMATION_BLOCK;
		_ani->reset();
		_ani->loopAni = false;
	}

	_health = healthCache;
	removeObject = removeObjectCache;

	return false;
}

void Gabriel::makeHurt()
{
	_ani = ANIMATION_HITLOW;
	_ani->reset();
	_ani->loopAni = false;
	_isMirrored = true;
	_health -= 1;
	removeObject = (_health <= 0);
}

void Gabriel::changeSwitch()
{
	_ani = ANIMATION_ACTION_CANNON;
	_ani->reset();
	_ani->loopAni = false;
	_isMirrored = false;
	_throwBombsTime = THROW_BOMBS_TIME; // throw bombs after `THROW_BOMBS_TIME` ms
	_canThrowBombs = true;
	_sendPiratesTime = SEND_PIRATES_TIME; // send pirates after `SEND_PIRATES_TIME` ms
	_canSendPirates = true;
}

GabrielCannon::GabrielCannon(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	_GabrielCannon = this;

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
GabrielCannon::~GabrielCannon()
{
	_GabrielCannon = nullptr;
}
void GabrielCannon::Logic(uint32_t elapsedTime)
{
	if (_ani->isFinishAnimation())
	{
		if (_ani == _vertfire)
		{
			setAnimation(_rest);
		}
		else if (_ani == _horzfire || _ani == _rest)
		{
			setAnimation(_home);
		}
	}

	_ani->Logic(elapsedTime);
}

void GabrielCannon::operateCannon()
{
	// shoot cannon ball
	WwdObject obj;
	obj.x = (int32_t)position.x;
	obj.y = (int32_t)position.y;
	obj.damage = 15;

	if (_ani == _rise)
	{
		setAnimation(_vertfire);
		obj.speedY = -DEFAULT_PROJECTILE_SPEED;
		_Gabriel->makeHurt();
	}
	else
	{
		setAnimation(_horzfire);
		obj.speedX = -DEFAULT_PROJECTILE_SPEED;
		obj.y += 56;
	}

	GO::addObjectToActionPlane(DBG_NEW CannonBall(obj));

	_Gabriel->changeSwitch();
}
void GabrielCannon::riseCannon()
{
	setAnimation(_rise);
}

void GabrielCannon::setAnimation(shared_ptr<UIAnimation> newAni){
	_ani = newAni;
	_ani->reset();
	_ani->loopAni = false;
	_ani->position = position;
}


GabrielCannonSwitch::GabrielCannonSwitch(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _switchCannonTime(OPERATE_CANNON_TIME)
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/SWITCH.ANI", PathManager::getImageSetPath(obj.imageSet));
	_ani->position = position;
	_ani->updateFrames = false;
}
void GabrielCannonSwitch::Logic(uint32_t elapsedTime)
{
	if (!_Gabriel) return;

	_switchCannonTime -= elapsedTime;
	if (_switchCannonTime <= 0) // use cannon every OPERATE_CANNON_TIME milliseconds
	{
		_switchCannonTime = OPERATE_CANNON_TIME;
		_GabrielCannon->operateCannon();
		_ani->reset();
		_ani->loopAni = false;
	}
	_ani->Logic(elapsedTime);
}


GabrielCannonButton::GabrielCannonButton(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	_idle = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/IDLE.ANI", PathManager::getImageSetPath(obj.imageSet));
	_pressed = AssetsManager::getAnimationFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");
	_ani = _idle;
	setObjectRectangle();
}
void GabrielCannonButton::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);

	if (_ani == _pressed && !_GabrielCannon->isUp())
	{
		_ani = _idle;
	}
	else if (GO::getPlayerRect().intersects(_objRc) || GO::getPlayerAttackRect().first.intersects(_objRc))
	{
		_ani = _pressed;
		_GabrielCannon->riseCannon();
	}
}
