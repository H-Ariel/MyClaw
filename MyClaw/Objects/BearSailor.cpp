#include "BearSailor.h"
#include "../Player.h"

/*
STRIKE1 - hand up
STRIKE2 - send hand to catch CC
STRIKE3 - catch CC
STRIKE4 - I couldn't figure it out
*/


#define ANIMATION_WALK			_animations.at(_walkAniName)
#define ANIMATION_IDLE			_animations.at("IDLE")
#define ANIMATION_STRIKE		_animations.at("STRIKE1")
#define ANIMATION_TRYCATCH		_animations.at("STRIKE2")
#define ANIMATION_HUG			_animations.at("STRIKE3")


BearSailor::BearSailor(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 14, 20, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL",
		"STRIKE1", "", "", "", "", "IDLE")
{
}

void BearSailor::Logic(uint32_t elapsedTime)
{
	BaseEnemy::Logic(elapsedTime);
	return;

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

	if (!_isStanding && !_isAttack)
	{
		position.x += _speed.x * elapsedTime;
		if (position.x < _minX) { stopMovingLeft(_minX - position.x); }
		else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); }
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

void BearSailor::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);

		if (deltaY < 24)
		{
			if (deltaX < 64) // CC is close to enemy
			{
				return; // TODO

				//_ani = ANIMATION_TRYCATCH;
				_ani = ANIMATION_HUG;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;
			}
			else if (deltaX < 96) // CC is little far from enemy
			{
				_ani = ANIMATION_STRIKE;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;
			}
		}
	}
}

D2D1_RECT_F BearSailor::GetRect()
{
	_saveCurrRect.left = position.x - 25;
	_saveCurrRect.right = position.x + 25;
	_saveCurrRect.top = position.y - 55;
	_saveCurrRect.bottom = position.y + 65;
	return _saveCurrRect;
}

pair<D2D1_RECT_F, int8_t> BearSailor::GetAttackRect()
{
	if (!_isAttack) return {};

	D2D1_RECT_F rc = {};

	if (_ani == ANIMATION_STRIKE)
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

		rc.top = 25;
		rc.bottom = 80;
	}
	else if (_ani == ANIMATION_HUG)
	{
		// TODO
	}

	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}
