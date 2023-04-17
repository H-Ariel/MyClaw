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
	ParrotTakeMarrow = 0, RemoveFloor = 0,
};

static GlobalState globalState = GlobalState::RemoveFloor;
static int floorCounter = 0; // use to sync floors

#define MARROW_ANIMATION_BLOCK _animations["BLOCK"]


Marrow::Marrow(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE1", "STRIKE2", "GAME/IMAGES/BULLETS")
{
	_health = 100;
	_ani = _animations["HOME"];
	_forward = false;
}

void Marrow::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;
	if (_ani == MARROW_ANIMATION_BLOCK)
	{
		if (_ani->isFinishAnimation())
		{
			_ani = _animations["HOME"];
		}
		else
		{
			PostLogic(elapsedTime);
			return;
		}
	}


	_blockClaw = (_hitsCuonter == 1 && !_blockClaw);

	if (_hitsCuonter == 5)
	{
		_hitsCuonter = 0;

		// TODO: open floor and move side (using parrot)
		

	}


	if (globalState == GlobalState::AddFloor)
	{
		if (floorCounter == 2)
		{
			// floor finish their job
			floorCounter = 0;
			globalState = GlobalState::ParrotAttackClaw;
		}
	}



	PostLogic(elapsedTime);
}

pair<Rectangle2D, int> Marrow::GetAttackRect()
{
	return {};
}

bool Marrow::checkForHurts()
{
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
			_hitsCuonter += 1;
			return true;
		}
	}

	return false;
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

	if (globalState == GlobalState::ParrotAttackClaw
		
		|| (
			globalState == GlobalState::ParrotReturnToMarrow &&
			position.x != _initialPosition.x &&
			abs(position.y - _initialPosition.y) > 5 
			)
		)
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


		if (_hitsCounter == 1) // todo: change to 3
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
	setObjectRectangle();
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
	}
	else if (globalState == GlobalState::RemoveFloor)
	{
		position.x += _speedX * elapsedTime;
		if (position.x < _minX)
		{
			position.x = _minX;
		}
		else if (position.x > _maxX)
		{
			position.x = _maxX;
		}
	}
	
}
