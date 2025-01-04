#include "Gabriel.h"
#include "RedTailPirate.h"
#include "../GlobalObjects.h"
#include "../Objects/EnemyProjectile.h"


static bool riseCannon = false;
static bool cannonIsUp = false;
static bool operateCannon = false;
static bool GabrielChangeSwitch = false;
static bool makeGabrielHurt = false;
static bool GabrielIsAlive = false;


#define ANIMATION_HITLOW		_animations["HITLOW"]
#define ANIMATION_SEND_PIRATES	_animations["STRIKE2"]
#define ANIMATION_ACTION_CANNON _animations["STRIKE3"]
#define ANIMATION_THROW_BOMBS	_animations["STRIKE7"]
#define ANIMATION_BLOCK			_animations["BLOCKED"]

#define OPERATE_CANNON_TIME	7000
#define THROW_BOMBS_TIME	4500
#define SEND_PIRATES_TIME	2000


class DeadGabriel : public OneTimeAnimation
{
public:
	DeadGabriel(D2D1_POINT_2F pos, shared_ptr<UIAnimation> ani, int firstPartDuration)
		: OneTimeAnimation(pos, ani, false), _delay(firstPartDuration), _moved(false)
	{
		drawZ = DefaultZCoord::Characters + 1;
	}
	void Logic(uint32_t elapsedTime) override
	{
		if (!_moved)
		{
			_delay -= elapsedTime;
			if (_delay <= 0)
			{
				// TODO: add flip in air before stay in this position
				_ani->position = { 42992, 5240 };
				_ani->mirrored = true;
				_moved = true;
			}
		}

		_ani->Logic(elapsedTime);
	}

private:
	int _delay;
	bool _moved;
};


Gabriel::Gabriel(const WwdObject& obj)
	: BaseBoss(obj)
	, _throwBombsTime(THROW_BOMBS_TIME), _canThrowBombs(true)
	, _sendPiratesTime(SEND_PIRATES_TIME), _canSendPirates(true)
{
	speed.x = 0;
	_isMirrored = true;
	_ani = _animations["IDLE"];
	_health = 5; // we need hurt Gabriel only 5 times to win
	GabrielIsAlive = true;
}
Gabriel::~Gabriel()
{
	GabrielIsAlive = false;
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW DeadGabriel(position,
			make_shared<UIAnimation>(
				_animations["KILLFALL1"]->getImagesList() +
				_animations["KILLFALL2"]->getImagesList() +
				_animations["KILLFALL3"]->getImagesList() +
				_animations["KILLFALL4"]->getImagesList()
			),
			(int)_animations["KILLFALL1"]->getTotalDuration()));
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

	if (GabrielChangeSwitch)
	{
		_ani = ANIMATION_ACTION_CANNON;
		_ani->reset();
		_ani->loopAni = false;
		_isMirrored = false;
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
		_isMirrored = true;
		makeGabrielHurt = false;
		_health -= 1;
		removeObject = (_health <= 0);
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

	int health = _health;

	if (BaseEnemy::checkForHurts())
	{
		_ani = ANIMATION_BLOCK;
		_ani->reset();
		_ani->loopAni = false;
	}

	_health = health;
	removeObject = false;

	return false;
}


GabrielCannon::GabrielCannon(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
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
	const UIAnimation* prevAni = _ani.get();

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

		GO::addObjectToActionPlane(DBG_NEW CannonBall(obj));

		operateCannon = false;
		GabrielChangeSwitch = true;
	}


	if (_ani.get() != prevAni)
	{
		_ani->reset();
		_ani->loopAni = false;
		_ani->position = position;
	}

	_ani->Logic(elapsedTime);
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
	if (!GabrielIsAlive) return;

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


GabrielCannonButton::GabrielCannonButton(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	_idle = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/IDLE.ANI", PathManager::getImageSetPath(obj.imageSet));
	_pressed = AssetsManager::createAnimationFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");
	_ani = _idle;
	setObjectRectangle();
}
void GabrielCannonButton::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);

	if (_ani == _pressed && !cannonIsUp)
	{
		_ani = _idle;
	}
	else if (GO::getPlayerRect().intersects(_objRc) || GO::getPlayerAttackRect().first.intersects(_objRc))
	{
		_ani = _pressed;
		riseCannon = true;
	}
}


GabrielRedTailPirate::GabrielRedTailPirate()
	: BaseDynamicPlaneObject({}), _isJumping(false)
{
	position = { 43479, 5020 };
	logicZ = DefaultZCoord::Characters;
	drawZ = DefaultZCoord::Characters;
	speed.x = -0.3f;
	_ani = AssetsManager::loadCopyAnimation("LEVEL8/ANIS/REDTAILPIRATE/ADVANCE.ANI", "LEVEL8/IMAGES/REDTAILPIRATE");
}
void GabrielRedTailPirate::Logic(uint32_t elapsedTime)
{
	speed.y += GRAVITY * elapsedTime;
	position.x += speed.x * elapsedTime;
	position.y += speed.y * elapsedTime;
	if (position.x < 43200 && !_isJumping)
	{
		speed.y = -0.7f;
		_isJumping = true;
		position.y -= 1;
		_ani->updateFrames = false;
	}
	_ani->position = position;
	_ani->Logic(elapsedTime);
}
void GabrielRedTailPirate::stopFalling(float collisionSize)
{
	if (_isJumping)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.minX = 42560;
		obj.maxX = 43200;
		obj.imageSet = "LEVEL_REDTAILPIRATE";
		GO::addObjectToActionPlane(DBG_NEW RedTailPirate(obj, true));

		removeObject = true;
	}
	else
	{
		BaseDynamicPlaneObject::stopFalling(collisionSize);
	}
}
