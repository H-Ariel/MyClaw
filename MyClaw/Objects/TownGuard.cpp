#include "TownGuard.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_STRIKE1	_animations.at("STRIKE1")
#define ANIMATION_STRIKE2	_animations.at("STRIKE2")
#define ANIMATION_IDLE1		_animations.at("IDLE1")
#define ANIMATION_IDLE2		_animations.at("IDLE2")


TownGuard::TownGuard(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "", "", "", "", {}),
	_type((Type)(obj.logic[obj.logic.length() - 1] - '1')), _attackRest(0)
{
	_ani = ANIMATION_IDLE1;
}

void TownGuard::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_isStanding)
	{
		if (_ani!= ANIMATION_IDLE1&&_ani!= ANIMATION_IDLE2)
		{
			_ani = getRandomInt(0, 1) ? ANIMATION_IDLE1 : ANIMATION_IDLE2;
			_ani->reset();
			_ani->loopAni = false;
		}
		if (_ani->isFinishAnimation())
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
	if (!_isAttack)
	{
		position.y += _speed.y * elapsedTime;
		_speed.y += GRAVITY * elapsedTime;
	}

	if (_attackRest > 0)
	{
		_attackRest -= elapsedTime;
	}
	if (!_isAttack && _attackRest <= 0)
	{
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
void TownGuard::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);

		if (deltaX < 96 && deltaY < 16) // CC is close to enemy
		{
			if (_player->isDuck()) _ani = ANIMATION_STRIKE1;
			else _ani = ANIMATION_STRIKE2;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;
			_attackRest = 750;
		}
	}
}
pair<D2D1_RECT_F, int8_t> TownGuard::GetAttackRect()
{
	D2D1_RECT_F rc = {};

	if (_isAttack)
	{
		if (_forward)
		{
			rc.left = 30;
			rc.right = 110;
		}
		else
		{
			rc.left = -60;
			rc.right = 20;
		}

		if (_ani == ANIMATION_STRIKE2)
		{
			rc.top = 20;
			rc.bottom = 40;
		}
		else
		{
			rc.top = 40;
			rc.bottom = 60;
		}
	}


	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage*0 };
}
