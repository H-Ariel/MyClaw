#include "Marrow.h"
#include "../Player.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"


// TODO: continue Marrow logic


enum class GlobalState : int8_t
{
	ParrotAttackClaw = 0,
	ParrotReturnToMarrow = 1, AddFloor = 1,
	ClawAttackMarrow = 2, ParrotIdle = 2,
	ParrotTakeMarrow = 3, RemoveFloor = 3
};

enum class MarrowSise : int8_t
{
	Left = 0,
	Right = 1
};


static GlobalState globalState = GlobalState::ParrotAttackClaw;
static int floorCounter = 0; // use to sync floors
static bool firstFloorsRemove = true;
static MarrowSise marrowSise = MarrowSise::Left;


#define MARROW_ANIMATION_BLOCK	_animations["BLOCK"]
#define MARROW_ANIMATION_HOME	_animations["HOME"]
#define MARROW_ANIMATION_HAND_UP		_animations["IDLE1"]
#define MARROW_ANIMATION_WAIT_HAND_UP	_animations["IDLE2"]
#define MARROW_ANIMATION_HAND_DOWN		_animations["IDLE4"]

/*
HOME - just stand
IDLE1 - raise his hand up
IDLE2 - wait with hand up (eyes open)
IDLE3 - wait with hand up (eyes closed)
IDLE4 - get hand down back
*/

Marrow::Marrow(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE1", "STRIKE2", "GAME/IMAGES/BULLETS")
{
	_health = 100;
	_ani = MARROW_ANIMATION_HOME;
	_forward = false;
}

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

	if (globalState == GlobalState::ClawAttackMarrow)
	{
		if (_hitsCuonter == 5)
		{
			_hitsCuonter = 0;

			// TODO: open floor and move side (using parrot)

			globalState = GlobalState::RemoveFloor;

			_ani = MARROW_ANIMATION_HAND_UP;
			_ani->reset();
			_ani->loopAni = false;
		}
	}
	else if (globalState == GlobalState::ParrotTakeMarrow)
	{
		if (_ani == MARROW_ANIMATION_HAND_UP && _ani->isFinishAnimation())
		{
			_speed.x = marrowSise == MarrowSise::Left ? -0.4f : 0.4f;
			_forward = marrowSise == MarrowSise::Left ? false : true;
			position.y -= 128;
			_ani = MARROW_ANIMATION_WAIT_HAND_UP;
			_ani->reset();
			_ani->loopAni = false;
		}
		else if (_ani == MARROW_ANIMATION_WAIT_HAND_UP && _speed.x == 0) // Marrow stops move
		{
			position.y += 128;
			marrowSise = marrowSise == MarrowSise::Left ? MarrowSise::Right : MarrowSise::Left;
			_forward = marrowSise == MarrowSise::Left ? false : true;
			globalState = GlobalState::AddFloor;
			_ani = MARROW_ANIMATION_HOME;
			_ani->reset();
		}

		position.x += _speed.x * elapsedTime;
	}
	

	if (!_isAttack)
	{
		if (globalState == GlobalState::ClawAttackMarrow)
			makeAttack();
	}
	else
	{
		if (_ani->isFinishAnimation())
		{
			_ani = MARROW_ANIMATION_HOME;
			_ani->reset();
			_isAttack = false;
		//	_forward = false;
		}
	}



	PostLogic(elapsedTime);
}
void Marrow::makeAttack()
{
	if (abs(_player->position.x - position.x) < 96 && abs(_player->position.y - position.y) < 16)
	{
		_ani = _animations["STRIKE2"]; // knife attack
		_ani->reset();
		_isAttack = true;
		_forward = _player->position.x > position.x;
	}
}

pair<Rectangle2D, int> Marrow::GetAttackRect()
{
	return {};
}

bool Marrow::checkForHurts()
{
	if (globalState != GlobalState::ClawAttackMarrow)
		return false;

	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isClawProjectile(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				_ani = MARROW_ANIMATION_BLOCK;
				_ani->reset();
				return false;
			}
		}
	}

	if (checkForHurt(_player->GetAttackRect()))
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
	_speed = {};
	_forward = false;
}
void Marrow::stopMovingRight(float collisionSize)
{
	BaseBoss::stopMovingRight(collisionSize);
	_speed = {};
	_forward = true;
}



#define PARROT_ANIMATION_STRIKE _animations["STRIKE1"]
#define MARROW_PARROT_SPEED 0.3f

MarrowParrot::MarrowParrot(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 3, 10, "FLY", "HIT", "HIT", "KILLFALL", "STRIKE1", "",
		"", "", "", MARROW_PARROT_SPEED, true), _hitsCounter(0), _initialPosition({}),
	_flyRect((float)obj.minX, (float)obj.minY - 32.f, (float)obj.maxX, (float)obj.maxY - 32.f)
{
	myMemCpy(_initialPosition, position);
	_forward = false;
	_speed = { 0, MARROW_PARROT_SPEED };
}

void MarrowParrot::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	position.x += _speed.x * elapsedTime;
	position.y += _speed.y * elapsedTime;

	if (globalState == GlobalState::ParrotAttackClaw ||
		(globalState == GlobalState::ParrotReturnToMarrow &&
			position.x != _initialPosition.x &&
			abs(position.y - _initialPosition.y) > 5))
	{
		if (position.y > _flyRect.bottom)
		{
			position.y = _flyRect.bottom;
			_speed = { -MARROW_PARROT_SPEED, 0 };
		}
		else if (position.y < _flyRect.top)
		{
			position.y = _flyRect.top;
			_speed = { MARROW_PARROT_SPEED, 0 };
		}
		else if (position.x < _flyRect.left)
		{
			position.x = _flyRect.left;
			_speed = { 0, -MARROW_PARROT_SPEED };
		}
		else if (position.x > _flyRect.right)
		{
			position.x = _flyRect.right;
			_speed = { 0, MARROW_PARROT_SPEED };
		}
		else if (_player->isTakeDamage()) // if player is hurt, parrot returns to Marrow
		{
			_speed = { 0, -MARROW_PARROT_SPEED };
		}


		if (_hitsCounter == 3) // todo: change to 3
		{
			_hitsCounter = 0;
			// TODO: return to initial position

			globalState = GlobalState::AddFloor;
		}
	}


	if (_speed.x < 0)
	{
		_ani = PARROT_ANIMATION_STRIKE;
		_isAttack = true;
	}
	else
	{
		_ani = _animations["FLY"];
		_isAttack = false;
	}
	
	
	
	
	
	

	_forward = _speed.x > 0;

	PostLogic(elapsedTime);
}

pair<Rectangle2D, int> MarrowParrot::GetAttackRect()
{
	return { GetRect(), _damage };
}

// the Parrot is flying so if it touch ground we ignore it
void MarrowParrot::stopFalling(float collisionSize) {}
void MarrowParrot::stopMovingLeft(float collisionSize) {}
void MarrowParrot::stopMovingRight(float collisionSize) {}
void MarrowParrot::bounceTop() {}

bool MarrowParrot::checkForHurts()
{
	// when CC hurt parrot, it returns to Marrow

	int health = _health; // save health value

	// TODO: hurt only from CC attack (not weapons)
	// i.e. if (checkForHurt(_player->GetAttackRect())) { ... }
	if (_isAttack && BaseEnemy::checkForHurts())
	{
		_hitsCounter += 1;
		_speed = { 0, -MARROW_PARROT_SPEED };
		_ani = _animations["HIT"];
		_ani->reset();
	}

	// make sure value did not changed
	_health = health;
	removeObject = false;

	return false;
}





MarrowFloor::MarrowFloor(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player),
	_minX((float)obj.minX), _maxX((float)obj.maxX),
	_speedX((obj.direction ? obj.speedX : -obj.speedX) / 1000.f)
{
	_ani = AssetsManager::createAnimationFromFromPidImage("LEVEL10/IMAGES/TRAPELEVATOR/001.PID");
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
			// floors finish their job
			floorCounter = 0;
			globalState = GlobalState::ClawAttackMarrow;
		}
	}
	else if (globalState == GlobalState::RemoveFloor || firstFloorsRemove)
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
