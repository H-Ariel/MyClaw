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


BearSailor::BearSailor(const WwdObject& obj)
	: BaseEnemy(obj, 14, 20, "FASTADVANCE", "HITHIGH",
		"HITLOW", "KILLFALL", "STRIKE1", "", "", "", "", ENEMY_PATROL_SPEED)
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
			_isMirrored = _speed.x < 0;
		}
	}

	PostLogic(elapsedTime);
}

void BearSailor::makeAttack()
{
	if (enemySeeClaw())
	{
		const float deltaX = abs(player->position.x - position.x), deltaY = abs(player->position.y - position.y);

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
				_isMirrored = player->position.x < position.x;
			}
			else if (deltaX < 96) // CC is little far from enemy
			{
				_ani = ANIMATION_STRIKE;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_isMirrored = player->position.x < position.x;
			}
		}
	}
}

Rectangle2D BearSailor::GetRect()
{
	_saveCurrRect.left = position.x - 25;
	_saveCurrRect.right = position.x + 25;
	_saveCurrRect.top = position.y - 55;
	_saveCurrRect.bottom = position.y + 65;
	return _saveCurrRect;
}

pair<Rectangle2D, int> BearSailor::GetAttackRect()
{
	if (_ani->getFrameNumber() != 3) return {};

	Rectangle2D rc;

	if (_ani == ANIMATION_STRIKE)
	{
		if (_isMirrored)
		{
			rc.left = -60;
			rc.right = 20;
		}
		else
		{
			rc.left = 30;
			rc.right = 110;
		}

		rc.top = 25;
		rc.bottom = 80;
	}
	else if (_ani == ANIMATION_HUG)
	{
		// TODO
	}

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}
