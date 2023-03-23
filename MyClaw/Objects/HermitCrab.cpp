#include "HermitCrab.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_SHOOT		_animations.at(_shootAniName)
#define ANIMATION_IDLE		_animations.at(_idleAniName)


HermitCrab::HermitCrab(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 1, 10, "FASTADVANCE", "HIT", "HIT", "KILLFALL",
		"STRIKE1", "", "STRIKE1", "", "", "IDLE"), _attackRest(0)
{
}

// this methods looks same to `BaseEnemy::Logic` ...
void HermitCrab::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_isStanding)
	{
		if (_ani != ANIMATION_IDLE)
		{
			_ani = ANIMATION_IDLE;
			_ani->reset();
		}
		else if (_ani->isFinishAnimation())
		{
			_isStanding = false;
			_ani = ANIMATION_WALK;
			_ani->reset();
		}
	}

	if (!_isStanding && !_isAttack && !_isStaticEnemy)
	{
		position.x += _speed.x * elapsedTime;
		if (position.x < _minX) { stopMovingLeft(_minX - position.x); }
		else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); }
	}

	if (_attackRest > 0)
		_attackRest -= elapsedTime;

	if (!_isAttack)
	{
		if (_isStaticEnemy)
		{
			_isStanding = true;
		}

		if (_attackRest <= 0)
			makeAttack();
	}
	else
	{
		if (_ani->isFinishAnimation())
		{
			_ani = ANIMATION_WALK;
			_ani->reset();
			_isAttack = false;
			_forward = _speed.x > 0;
		}
	}

	PostLogic(elapsedTime);
}

void HermitCrab::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x);
		const float deltaY = abs(_player->position.y - position.y);

		if (deltaX < 256 && deltaY < 42)
		{
			_ani = ANIMATION_SHOOT;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
			obj.y = (int32_t)position.y - 24;
			obj.z = ZCoord;
			obj.speedX = _forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
			ActionPlane::addPlaneObject(DBG_NEW CrabBomb(obj));

			_attackRest = 1200;
		}
	}
}

pair<D2D1_RECT_F, int8_t> HermitCrab::GetAttackRect() { return {}; }
bool HermitCrab::isDuck() const { return true; }
bool HermitCrab::isTakeDamage() const { return false; }
