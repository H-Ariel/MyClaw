#include "Marrow.h"
#include "MarrowStates.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/Projectiles/ClawProjectile.h"


#define MARROW_ANIMATION_BLOCK	_animations["BLOCK"]
#define MARROW_ANIMATION_HOME	_animations["HOME"] // just stand

#define PARROT_ANIMATION_STRIKE	_animations["STRIKE1"]

constexpr int MAX_FLOORS_COUNT = 2;


Marrow* Marrow::_MarrowInstance = nullptr;
int8_t MarrowFloor::floorCounter = 0;


Marrow::Marrow(const WwdObject& obj)
	: BaseBoss(obj, "KILLFALL"), _side(true) // start on the right side
{
	speed.x = 0;
	_health = 100;
	_ani = MARROW_ANIMATION_HOME;
	_isMirrored = true;
	_MarrowInstance = this;
	_state = DBG_NEW FirstFloorsRemoveState();
}
Marrow::~Marrow() { delete _state; _MarrowInstance = nullptr; }
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
	_state->MarrowLogic(this, elapsedTime);

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
		if (_state->getType() == MarrowState::StateType::ClawAttackMarrow)
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
		_isMirrored = GO::getPlayerPosition().x < position.x;
	}
}
bool Marrow::checkForHurts()
{
	if (_state->getType() == MarrowState::StateType::ClawAttackMarrow) // Marrow can hurt only in this state
	{
		for (Projectile* p : GO::getActionPlaneProjectiles())
			if (isinstance<ClawProjectile>(p))
				if (_saveCurrRect.intersects(p->GetRect()))
				{
					_ani = MARROW_ANIMATION_BLOCK;
					_ani->reset();
					return false;
				}

		if (checkClawHit())
			if (_blockClaw)
			{
				_ani = MARROW_ANIMATION_BLOCK;
				return false;
			}
			else
			{
				if (isinstance<ClawAttackMarrowState>(_state))
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
void Marrow::switchState(MarrowState* newState) {
	delete _state;
	_state = newState;
}


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

	if (Marrow::getInstance()) {
		position.x += speed.x * elapsedTime;
		position.y += speed.y * elapsedTime;

		Marrow::getInstance()->getState()->ParrotLogic(this, elapsedTime);

		if ((speed.x > 0 && Marrow::getInstance()->isInLefttSide()) ||
			(speed.x < 0 && Marrow::getInstance()->isInRightSide()))
		{
			_ani = PARROT_ANIMATION_STRIKE;
			_isAttack = true;
		}
		else
		{
			_ani = _animations["FLY"];
			_isAttack = false;
		}

		if (Marrow::getInstance()->getState()->getType() != MarrowState::StateType::ParrotTakeMarrow)
			_isMirrored = speed.x < 0;
	}

	PostLogic(elapsedTime);
}
pair<Rectangle2D, int> MarrowParrot::GetAttackRect() { return { GetRect(), _damage }; }
bool MarrowParrot::checkForHurts()
{
	int health = _health; // save health value

	if (_isAttack && checkForHurt(GO::getPlayerAttackRect()))
	{
		_hitsCounter += 1;
		speed = { 0, -PARROT_SPEED };
		_ani = _animations["HIT"];
		_ani->reset();
	}

	// make sure value did not changed (parrot could not die)
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
	: BaseStaticPlaneObject(obj, true),
	_minX((float)obj.minX), _maxX((float)obj.maxX),
	_speedX((obj.direction ? obj.speedX : -obj.speedX) / 1000.f)
{
	_ani = AssetsManager::getAnimationFromPidImage("LEVEL10/IMAGES/TRAPELEVATOR/001.PID");
}
void MarrowFloor::Logic(uint32_t elapsedTime)
{
	tryCatchPlayer();
	if (Marrow::getInstance())
		Marrow::getInstance()->getState()->FloorLogic(this, elapsedTime);
}
Rectangle2D MarrowFloor::GetRect()
{
	return BasePlaneObject::GetRect();
}

void MarrowFloor::moveFloor(uint32_t elapsedTime, bool isAddFloor) {
	
	float delta = _speedX * elapsedTime;
	if (isAddFloor)
		position.x -= delta;
	else
		position.x += delta;

	if (position.x < _minX)
	{
		position.x = _minX;
		MarrowFloor::floorCounter += 1;
	}
	else if (position.x > _maxX)
	{
		position.x = _maxX;
		MarrowFloor::floorCounter += 1;
	}

	if (MarrowFloor::floorCounter == MAX_FLOORS_COUNT) // all floors finished their job
	{
		MarrowFloor::floorCounter = 0;
		
		if (isAddFloor) {
			Marrow::getInstance()->switchState(DBG_NEW ClawAttackMarrowState());
		}
		else {
			if (Marrow::getInstance()->getState()->getType() == MarrowState::StateType::FirstFloorsRemove)
				Marrow::getInstance()->switchState(DBG_NEW ParrotAttackClawState());
		}
	}
}
