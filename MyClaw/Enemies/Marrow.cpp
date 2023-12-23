#include "Marrow.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define MARROW_ANIMATION_BLOCK			_animations["BLOCK"]
#define MARROW_ANIMATION_HOME			_animations["HOME"] // just stand
#define MARROW_ANIMATION_HAND_UP		_animations["IDLE1"] // raise his hand up
#define MARROW_ANIMATION_WAIT_HAND_UP	_animations["IDLE2"] // wait with hand up (eyes open)
#define MARROW_ANIMATION_HAND_DOWN		_animations["IDLE4"] // get hand down back

#define PARROT_ANIMATION_STRIKE _animations["STRIKE1"]
#define PARROT_SPEED 0.4f


enum class GlobalState : int8_t
{
	ParrotAttackClaw = 0,
	AddFloor = 1,
	ClawAttackMarrow = 2,
	ParrotTakeMarrow = 3
};


static Marrow* _Marrow = nullptr;
static int8_t floorCounter = 0; // use to sync floors
static GlobalState globalState = GlobalState::ParrotAttackClaw;
static bool firstFloorsRemove = true; // flag to say if this is the first time we remove floors (when CC enter to boss area)


Marrow::Marrow(const WwdObject& obj)
	: BaseBoss(obj, "KILLFALL"), _side(Side::Right)
{
	speed.x = 0;
	_health = 100;
	_ani = MARROW_ANIMATION_HOME;
	_isMirrored = true;
	_Marrow = this;
}
Marrow::~Marrow() { _Marrow = nullptr; }
void Marrow::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;
	if (_ani == MARROW_ANIMATION_BLOCK)
	{
		if (_ani->isFinishAnimation())
		{
			_ani = MARROW_ANIMATION_HOME;
		}
		else
		{
			PostLogic(elapsedTime);
			return;
		}
	}


	_blockClaw = (_hitsCuonter == 1 && !_blockClaw);

	switch (globalState)
	{
	case GlobalState::ClawAttackMarrow:
		if (_hitsCuonter == 5)
		{
			_hitsCuonter = 0;
			globalState = GlobalState::ParrotTakeMarrow;
			_ani = MARROW_ANIMATION_HAND_UP;
			_ani->reset();
			_ani->loopAni = false;
		}
		break;

	case GlobalState::ParrotTakeMarrow:
		if (_ani == MARROW_ANIMATION_HAND_UP && _ani->isFinishAnimation())
		{
			_isMirrored = _side == Marrow::Side::Right;
			speed.x = _isMirrored ? -0.4f : 0.4f;
			position.y -= 128;
			_ani = MARROW_ANIMATION_WAIT_HAND_UP;
			_ani->reset();
			_ani->loopAni = false;
		}
		else if (_ani == MARROW_ANIMATION_WAIT_HAND_UP && speed.x == 0) // Marrow stops move
		{
			position.y += 128;
			_isMirrored = _side == Marrow::Side::Right;
			_side = _isMirrored ? Marrow::Side::Left : Marrow::Side::Right;
			globalState = GlobalState::ParrotAttackClaw;
			_ani = MARROW_ANIMATION_HOME;
			_ani->reset();
		}

		position.x += speed.x * elapsedTime;

		break;

	default:
		break;
	}


	if (_isAttack)
	{
		if (_ani->isFinishAnimation())
		{
			_ani = MARROW_ANIMATION_HOME;
			_ani->reset();
			_isAttack = false;
		}
	}
	else
	{
		if (globalState == GlobalState::ClawAttackMarrow)
			BaseBoss::makeAttack();
	}


	PostLogic(elapsedTime);
}
void Marrow::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 96 && deltaY < 16)
	{
		_ani = _animations["STRIKE2"]; // knife attack
		_ani->reset();
		_isAttack = true;
		_isMirrored = player->position.x < position.x;
	}
}
bool Marrow::checkForHurts()
{
	if (globalState != GlobalState::ClawAttackMarrow)
		return false;

	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isbaseinstance<ClawProjectile>(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				_ani = MARROW_ANIMATION_BLOCK;
				_ani->reset();
				return false;
			}
		}
	}

	if (checkClawHit())
	{
		if (_blockClaw)
		{
			_ani = MARROW_ANIMATION_BLOCK;
			return false;
		}
		else
		{
			if (globalState == GlobalState::ClawAttackMarrow)
				_hitsCuonter += 1;
			return true;
		}
	}

	return false;
}
void Marrow::stopMovingLeft(float collisionSize)
{
	BaseBoss::stopMovingLeft(collisionSize);
	speed = {};
	_isMirrored = true;
}
void Marrow::stopMovingRight(float collisionSize)
{
	BaseBoss::stopMovingRight(collisionSize);
	speed = {};
	_isMirrored = false;
}
pair<Rectangle2D, int> Marrow::GetAttackRect() { return {}; }


MarrowParrot::MarrowParrot(const WwdObject& obj)
	: BaseEnemy(obj, 3, 10, "FLY", "HIT", "HIT", "KILLFALL", "STRIKE1", "",
		"", "", "", PARROT_SPEED, true), _hitsCounter(0), _initialPosition({}),
	_flyRect((float)obj.minX, (float)obj.minY - 32.f, (float)obj.maxX, (float)obj.maxY - 32.f)
{
	myMemCpy(_initialPosition, position);
	_isMirrored = true;
	speed = { 0, PARROT_SPEED };
}
void MarrowParrot::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;
	if (!_Marrow) { PostLogic(elapsedTime); return; }

	position.x += speed.x * elapsedTime;
	position.y += speed.y * elapsedTime;

	bool speedChanged = true;

	switch (globalState)
	{
	case GlobalState::ParrotAttackClaw:
		if (position.y > _flyRect.bottom)
		{
			position.y = _flyRect.bottom;
			speed = { -PARROT_SPEED, 0 };
		}
		else if (position.y < _flyRect.top)
		{
			position.y = _flyRect.top;
			speed = { PARROT_SPEED, 0 };
		}
		else if (position.x < _flyRect.left)
		{
			position.x = _flyRect.left;
			speed = { 0, -PARROT_SPEED };
		}
		else if (position.x > _flyRect.right)
		{
			position.x = _flyRect.right;
			speed = { 0, PARROT_SPEED };
		}
		else if (player->isTakeDamage()) // if player is hurt, parrot returns to Marrow
		{
			speed = { 0, -PARROT_SPEED };
		}
		else speedChanged = false;

		if (speedChanged && _Marrow->getSide() == Marrow::Side::Left)
		{
			speed.x = -speed.x;
			speed.y = -speed.y;
		}


		if (_hitsCounter == 3)
		{
			_hitsCounter = 0;
			globalState = GlobalState::AddFloor;
		}
		break;

	case GlobalState::ClawAttackMarrow:
		position = _initialPosition;
		break;

	case GlobalState::ParrotTakeMarrow:
		_ani = _animations["FLY"];
		_isAttack = false;
		_isMirrored = _Marrow->isMirrored();
		position = _Marrow->position;
		position.y -= 64;
		if (_isMirrored) position.x -= 48;
		else position.x += 48;
		break;

	default:
		break;
	}

	if ((speed.x > 0 && _Marrow->getSide() == Marrow::Side::Left) ||
		(speed.x < 0 && _Marrow->getSide() == Marrow::Side::Right))
	{
		_ani = PARROT_ANIMATION_STRIKE;
		_isAttack = true;
	}
	else
	{
		_ani = _animations["FLY"];
		_isAttack = false;
	}

	if (globalState != GlobalState::ParrotTakeMarrow)
		_isMirrored = speed.x < 0;

	PostLogic(elapsedTime);
}
pair<Rectangle2D, int> MarrowParrot::GetAttackRect() { return { GetRect(), _damage }; }
bool MarrowParrot::checkForHurts()
{
	int health = _health; // save health value

	if (_isAttack && checkForHurt(player->GetAttackRect()))
	{
		_hitsCounter += 1;
		speed = { 0, -PARROT_SPEED };
		_ani = _animations["HIT"];
		_ani->reset();
	}

	// make sure value did not changed
	_health = health;
	removeObject = false;

	return false;
}
// the Parrot is flying so if it touch ground we ignore it
void MarrowParrot::stopFalling(float collisionSize) {}
void MarrowParrot::stopMovingLeft(float collisionSize) {}
void MarrowParrot::stopMovingRight(float collisionSize) {}
void MarrowParrot::bounceTop() {}


MarrowFloor::MarrowFloor(const WwdObject& obj)
	: BaseStaticPlaneObject(obj),
	_minX((float)obj.minX), _maxX((float)obj.maxX),
	_speedX((obj.direction ? obj.speedX : -obj.speedX) / 1000.f)
{
	_ani = AssetsManager::createAnimationFromPidImage("LEVEL10/IMAGES/TRAPELEVATOR/001.PID");
}
void MarrowFloor::Logic(uint32_t elapsedTime)
{
	tryCatchPlayer();

	if (globalState == GlobalState::AddFloor)
	{
		position.x -= _speedX * elapsedTime;
		if (position.x < _minX)
		{
			position.x = _minX;
			floorCounter += 1;
		}
		else if (position.x > _maxX)
		{
			position.x = _maxX;
			floorCounter += 1;
		}

		if (floorCounter == 2)
		{
			// floors finished their job
			floorCounter = 0;
			globalState = GlobalState::ClawAttackMarrow;
		}
	}
	else if (globalState == GlobalState::ParrotTakeMarrow || firstFloorsRemove) // when parrot take Marrow, floors remove
	{
		position.x += _speedX * elapsedTime;
		if (position.x < _minX)
		{
			position.x = _minX;
			floorCounter += 1;
		}
		else if (position.x > _maxX)
		{
			position.x = _maxX;
			floorCounter += 1;
		}

		if (floorCounter == 2)
		{
			floorCounter = 0;
			firstFloorsRemove = false;
		}
	}
}
Rectangle2D MarrowFloor::GetRect()
{
	return BasePlaneObject::GetRect();
}
