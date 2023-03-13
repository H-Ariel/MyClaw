#include "LeRauxe.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_STRIKE	_animations.at("STRIKE")
#define ANIMATION_STAB		_animations.at("STAB")
#define ANIMATION_BLOCKHIGH	_animations.at("BLOCKHIGH")
#define ANIMATION_BLOCKLOW	_animations.at("BLOCKLOW")
#define ANIMATION_JUMP		_animations.at("JUMPBACK")


LeRauxe::LeRauxe(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 100, 10, "ADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE", "", "", {}),
	_attackRest(0), _hitsCuonter(1), _blockClaw(false), _canJump(true)
{
}

void LeRauxe::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;


	const shared_ptr<Animation> prevAni = _ani;

	if (_hitsCuonter == 0 && _canJump)
	{
		_ani = ANIMATION_JUMP;

		/*
		y = 0.5 * a * t^2 + v0 * t + y0
		  
		v0 = ?
		a  = GRAVITY
		y0 = 0
		y  = 128
		t  = 150
		
		128 = 0.5 * GRAVITY * 22500 + v0 * 150 + 0
		...
		*/
		_speed.y = -(0.85f - 75 * GRAVITY);
		_speed.x = (position.x - _minX > _maxX - position.x) ? -0.35f : 0.35f;

		_isAttack = false;
		_canJump = false;
	}
	_blockClaw = (_hitsCuonter == 1 && !_blockClaw);

	if (!_isAttack)
	{
		position.x += _speed.x * elapsedTime;
		if (position.x < _minX)
		{
			stopMovingLeft(_minX - position.x);
		}
		else if (position.x > _maxX)
		{
			stopMovingRight(position.x - _maxX);
		}
	}
	position.y += _speed.y * elapsedTime;
	_speed.y += GRAVITY * elapsedTime;

	if (_attackRest > 0)
	{
		_attackRest -= elapsedTime;
	}
	else if (!_isAttack && _attackRest <= 0 && _ani != ANIMATION_JUMP)
	{
		makeAttack();
	}
	else if (_ani->isFinishAnimation())
	{
		_ani = ANIMATION_WALK;
		_isAttack = false;
		_forward = _speed.x > 0;
	}
	
	if (_ani != ANIMATION_JUMP && abs(_player->position.x - position.x) > 64)
	{
		_forward = _player->position.x > position.x;
		if (_forward) _speed.x = abs(_speed.x);
		else _speed.x = -abs(_speed.x);
	}

	if (_ani != prevAni)
	{
		_ani->reset();
	}


	PostLogic(elapsedTime);
}
D2D1_RECT_F LeRauxe::GetRect()
{
	D2D1_RECT_F rc = _ani->GetRect();

	rc.left = -7.f + 15 * (!_forward);
	rc.right = rc.left + 50;
	rc.top = 5;
	rc.bottom = 115;

	// set rectangle by center
	const float addX = position.x - (rc.right - rc.left) / 2, addY = position.y - (rc.bottom - rc.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	_saveCurrRect = rc;

	return rc;
}
pair<D2D1_RECT_F, int8_t> LeRauxe::GetAttackRect()
{
	D2D1_RECT_F rc = {};

	if (_ani == ANIMATION_STRIKE)
	{
		rc.top = 60;
		rc.bottom = 80;

		if (_forward)
		{
			rc.left = 50;
			rc.right = 130;
		}
		else
		{
			rc.left = -80;
			rc.right = 0;
		}
	}
	else if (_ani == ANIMATION_STAB)
	{
		rc.top = 30;
		rc.bottom = 50;

		if (_forward)
		{
			rc.left = 50;
			rc.right = 130;
		}
		else
		{
			rc.left = -80;
			rc.right = 0;
		}
	}

	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;


	return { rc, _damage };
}

void LeRauxe::stopFalling(float collisionSize)
{
	_speed.y = 0;
	position.y -= collisionSize;
	
	if (_ani == ANIMATION_JUMP)
	{
		stopMovingLeft(0);
		_ani = ANIMATION_WALK;
	}
}
void LeRauxe::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;
	_speed.x = ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_forward = true;
}
void LeRauxe::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	_speed.x = -ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_forward = false;
}

bool LeRauxe::PreLogic(uint32_t elapsedTime)
{
	if (_ani == ANIMATION_JUMP)
	{
		if (_ani->isFinishAnimation())
		{
			stopMovingLeft(0);
		}
		else
		{
			position.x += _speed.x * elapsedTime;
			position.y += _speed.y * elapsedTime;
			_speed.y += GRAVITY * elapsedTime;

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
void LeRauxe::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);
		if (deltaX < 96 && deltaY < 16) // CC is close to enemy
		{
			if (_canStrike)
			{
				if (_player->isDuck()) _ani = ANIMATION_STRIKE;
				else _ani = ANIMATION_STAB;
				_ani->reset();
				_isAttack = true;
				_forward = _player->position.x > position.x;

				_attackRest = 500;
			}
		}
	}
}
bool LeRauxe::checkForHurts()
{
	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isClawProjectile(p))
		{
			if (CollisionDistances::isCollision(_saveCurrRect, p->GetRect()))
			{
				if (_player->isDuck()) _ani = ANIMATION_BLOCKLOW;
				else _ani = ANIMATION_BLOCKHIGH;
				_ani->reset();
				return false;
			}
		}
	}

	if (checkForHurt(_player->GetAttackRect()))
	{
		if (_blockClaw)
		{
			if (_player->isDuck()) _ani = ANIMATION_BLOCKLOW;
			else _ani = ANIMATION_BLOCKHIGH;
			return false;
		}
		else
		{
			// jump/block every 3 hits
			_hitsCuonter = (_hitsCuonter + 1) % 3;
			_canJump = true;
			return true;
		}
	}

	return false;
}
