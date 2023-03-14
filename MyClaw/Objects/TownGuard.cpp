#include "TownGuard.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK			_animations.at(_walkAniName)
#define ANIMATION_STRIKE_LOW	_animations.at("STRIKE1")
#define ANIMATION_STRIKE_HIGH	_animations.at("STRIKE2")
#define ANIMATION_IDLE1			_animations.at("IDLE1")
#define ANIMATION_IDLE2			_animations.at("IDLE2")


TownGuard::TownGuard(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "", "", "", "", {}),
	_type((Type)(obj.logic[obj.logic.length() - 1] - '1'))
{
	_ani = ANIMATION_IDLE1;
}

void TownGuard::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_isStanding)
	{
		if (_ani != ANIMATION_IDLE1 && _ani != ANIMATION_IDLE2)
		{
			_ani = getRandomInt(0, 1) ? ANIMATION_IDLE1 : ANIMATION_IDLE2;
			_ani->reset();
			_ani->loopAni = false;
		}
		if (_ani->isFinishAnimation() && !_isStaticEnemy)
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

	if (!_isAttack)
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
			if (_player->isDuck()) _ani = ANIMATION_STRIKE_LOW;
			else _ani = ANIMATION_STRIKE_HIGH;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;
		}
	}
}
D2D1_RECT_F TownGuard::GetRect()
{
	D2D1_RECT_F rc = {};

	if (_type == Type::Guard1)
	{
		rc.left = position.x - 20;
		rc.right = position.x + 20;
		rc.top = position.y - 50;
		rc.bottom = position.y + 60;
	}
	if (_type == Type::Guard2)
	{
		rc.top = position.y - 50;
		rc.bottom = position.y + 70;

		if (_forward)
		{
			rc.left = position.x - 30;
			rc.right = position.x + 10;
		}
		else
		{
			rc.left = position.x - 10;
			rc.right = position.x + 30;
		}
	}

	_saveCurrRect = rc;
	return rc;
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

		if (_ani == ANIMATION_STRIKE_HIGH)
		{
			if (_type == Type::Guard1)
			{
				rc.top = 20;
				rc.bottom = 40;
			}
			else
			{
				rc.top = 30;
				rc.bottom = 50;
			}
		}
		else
		{
			if (_type == Type::Guard1)
			{
				rc.top = 50;
				rc.bottom = 70;
			}
			else
			{
				rc.top = 80;
				rc.bottom = 100;
			}
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
