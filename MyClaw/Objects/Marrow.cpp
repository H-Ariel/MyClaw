#include "Marrow.h"
#include "../Player.h"
#include "../AssetsManager.h"


static bool openFloor = false;


Marrow::Marrow(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE1", "STRIKE2", "GAME/IMAGES/BULLETS")
{
	_ani = _animations["HOME"];
	_forward = false;
}

void Marrow::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;
	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> Marrow::GetAttackRect()
{
	return {};
}

bool Marrow::checkForHurts()
{
	return false;
}





#define PARROT_ANIMATION_STRIKE _animations["STRIKE1"]
#define MARROW_PARROT_SPEED 0.3f

MarrowParrot::MarrowParrot(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 3, 10, "FLY", "HIT", "HIT", "KILLFALL", "STRIKE1", "",
		"", "", "", MARROW_PARROT_SPEED, true), _state(State::Fly), _hitsCounter(0),
	_flyRect((float)obj.minX, (float)obj.minY - 32.f, (float)obj.maxX, (float)obj.maxY - 32.f)
{
	//	_ani = _animations["FLY"];
	_forward = false;

	_speed = { 0, MARROW_PARROT_SPEED };
}

void MarrowParrot::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime))
		return;

	position.x += _speed.x * elapsedTime;
	position.y += _speed.y * elapsedTime;


	if (_state == State::Fly)
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
	
	
	if (_hitsCounter == 3)
	{
		_hitsCounter = 0;
		// TODO: return to initial position
		
		// open floor
		openFloor = true;
	}
	
	
	

	_forward = _speed.x > 0;

	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> MarrowParrot::GetAttackRect()
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

	int16_t health = _health; // save health value

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
	: BaseStaticPlaneObject(obj, player), _isOpen(false),
	_minX((float)obj.minX), _maxX((float)obj.maxX), _speedX(obj.speedX / 1000.f)
{
	_ani = AssetsManager::createAnimationFromFromPidImage("LEVEL10/IMAGES/TRAPELEVATOR/001.PID");
}

void MarrowFloor::Logic(uint32_t elapsedTime)
{
	if (position.x < _minX )
	{
		position.x = _minX;
		_speedX = -_speedX;
	}
	else if (position.x > _maxX)
	{
		position.x = _maxX;
		_speedX = -_speedX;
	}

	position.x += _speedX * elapsedTime;
}
