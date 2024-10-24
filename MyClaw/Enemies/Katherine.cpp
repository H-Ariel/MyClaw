#include "Katherine.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at("WALK")
#define ANIMATION_STRIKE1	_animations.at("STRIKE1")
#define ANIMATION_STRIKE2	_animations.at("STRIKE2")
#define ANIMATION_BLOCKHIGH	_animations.at("BLOCKHIGH")
#define ANIMATION_BLOCKLOW	_animations.at("BLOCKLOW")
#define ANIMATION_FLIP		_animations.at("FLIP3")


Katherine::Katherine(const WwdObject& obj)
	: BaseBoss(obj, "FALL")
{
}

void Katherine::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;


	const UIAnimation* prevAni = _ani.get();

	if (_hitsCuonter == 0 && _canJump)
	{
		_ani = ANIMATION_FLIP;

		speed.y = -(0.512f + GRAVITY * 125);
		speed.x = (((position.x - _minX < _maxX - position.x) ? _maxX : _minX) - position.x) / 750;

		_isAttack = false;
		_canJump = false;
	}
	_blockClaw = (_hitsCuonter == 1 && !_blockClaw);

	if (!_isAttack)
	{
		position.x += speed.x * elapsedTime;
		if (position.x < _minX)
		{
			stopMovingLeft(_minX - position.x);
		}
		else if (position.x > _maxX)
		{
			stopMovingRight(position.x - _maxX);
		}
	}
	speed.y += GRAVITY * elapsedTime;
	position.y += speed.y * elapsedTime;

	if (!_isAttack && _attackRest <= 0 && _ani != ANIMATION_FLIP)
	{
		BaseBoss::makeAttack();
	}
	else if (_ani->isFinishAnimation())
	{
		_ani = ANIMATION_WALK;
		_isAttack = false;
		if (speed.x == 0)
		{
			if (_isMirrored) speed.x = -ENEMY_PATROL_SPEED;
			else speed.x = ENEMY_PATROL_SPEED;
		}
		else
			_isMirrored = speed.x < 0;
	}

	if (_ani != ANIMATION_FLIP && abs(player->position.x - position.x) > 64)
	{
		_isMirrored = player->position.x < position.x;
		if (_isMirrored) speed.x = -abs(speed.x);
		else speed.x = abs(speed.x);
	}

	if (_ani.get() != prevAni)
	{
		_ani->reset();
	}


	PostLogic(elapsedTime);
}
Rectangle2D Katherine::GetRect()
{
	Rectangle2D rc;

	rc.left = -7.f + 15 * _isMirrored;
	rc.right = rc.left + 50;
	rc.top = 5;
	rc.bottom = 115;

	_saveCurrRect = setRectByCenter(rc);

	return _saveCurrRect;
}
pair<Rectangle2D, int> Katherine::GetAttackRect()
{
	Rectangle2D rc;

	if (_ani == ANIMATION_STRIKE1)
	{
		rc.top = 50;
		rc.bottom = 70;

		if (_isMirrored)
		{
			rc.left = -148;
			rc.right = -128;
		}
		else
		{
			rc.left = 182;
			rc.right = 202;
		}
	}
	else if (_ani == ANIMATION_STRIKE2)
	{
		rc.top = 10;
		rc.bottom = 40;

		if (_isMirrored)
		{
			rc.left = -25;
			rc.right = 0;
		}
		else
		{
			rc.left = 50;
			rc.right = 75;
		}
	}
	else return {};

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}

void Katherine::stopFalling(float collisionSize)
{
	speed.y = 0;
	position.y -= collisionSize;

	if (_ani == ANIMATION_FLIP)
	{
		stopMovingLeft(0);
		_ani = ANIMATION_WALK;
	}
}
void Katherine::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;
	speed.x = ENEMY_PATROL_SPEED;
	speed.y = 0;
	_isMirrored = false;
}
void Katherine::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	speed.x = -ENEMY_PATROL_SPEED;
	speed.y = 0;
	_isMirrored = true;
}

bool Katherine::PreLogic(uint32_t elapsedTime)
{
	if (_attackRest > 0)
	{
		_attackRest -= elapsedTime;
	}

	if (_ani == ANIMATION_FLIP)
	{
		if (_ani->isFinishAnimation())
		{
			stopMovingLeft(0);
		}
		else
		{
			position.x += speed.x * elapsedTime;
			speed.y += GRAVITY * elapsedTime;
			position.y += speed.y * elapsedTime;

			PostLogic(elapsedTime);
			return false;
		}
	}

	if (!BaseBoss::PreLogic(elapsedTime))
		return false;

	if (_ani == ANIMATION_BLOCKHIGH || _ani == ANIMATION_BLOCKLOW)
	{
		if (_ani->isFinishAnimation())
		{
			_ani = ANIMATION_WALK;
			stopMovingLeft(0);
		}
		else
		{
			PostLogic(elapsedTime);
			return false;
		}
	}

	return true;
}
void Katherine::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 48 && deltaY < 16) // CC is close to K
	{
		_ani = ANIMATION_STRIKE2;
		_ani->reset();
		_isAttack = true;
		_isMirrored = player->position.x < position.x;

		_attackRest = 800;
	}
	else if (192 < deltaX && deltaX < 208 && deltaY < 24) // CC is far from K
	{
		_ani = ANIMATION_STRIKE1;
		_ani->reset();
		_isAttack = true;
		_isMirrored = player->position.x < position.x;

		_attackRest = 1200;
	}
}
bool Katherine::checkForHurts()
{
	for (Projectile* p : actionPlane->getProjectiles())
	{
		if (isinstance<ClawProjectile>(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				if (player->isDuck()) _ani = ANIMATION_BLOCKLOW;
				else _ani = ANIMATION_BLOCKHIGH;
				_ani->reset();
				return false;
			}
		}
	}

	if (checkClawHit())
	{
		if (_blockClaw)
		{
			if (player->isDuck()) _ani = ANIMATION_BLOCKLOW;
			else _ani = ANIMATION_BLOCKHIGH;
			return false;
		}
		else
		{
			// jump/block every 4 hits
			_hitsCuonter = (_hitsCuonter + 1) % 4;
			_canJump = true;
			return true;
		}
	}

	return false;
}
