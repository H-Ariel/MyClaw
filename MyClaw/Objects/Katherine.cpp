#include "Katherine.h"
#include "../ActionPlane.h"
#include "../Player.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_STRIKE1	_animations.at("STRIKE1")
#define ANIMATION_STRIKE2	_animations.at("STRIKE2")
#define ANIMATION_BLOCKHIGH	_animations.at("BLOCKHIGH")
#define ANIMATION_BLOCKLOW	_animations.at("BLOCKLOW")
#define ANIMATION_FLIP		_animations.at("FLIP3")


Katherine::Katherine(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "WALK", "HITHIGH", "HITLOW",
		"FALL", "", "", "", {}),
	_attackRest(0), _hitsCuonter(1), _blockClaw(false), _canJump(true)
{
	myMemCpy(_minX, position.x - 320);
	myMemCpy(_maxX, position.x + 320);
}

void Katherine::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;


	const shared_ptr<Animation> prevAni = _ani;

	if (_hitsCuonter == 0 && _canJump)
	{
		_ani = ANIMATION_FLIP;

		_speed.y = -(0.512f + GRAVITY * 125);
		_speed.x = (((position.x - _minX < _maxX - position.x) ? _maxX : _minX) - position.x) / 750;

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

	if (!_isAttack && _attackRest <= 0 && _ani != ANIMATION_FLIP)
	{
		makeAttack();
	}
	else if (_ani->isFinishAnimation())
	{
		_ani = ANIMATION_WALK;
		_isAttack = false;
		_forward = _speed.x > 0;
	}

	if (_ani != ANIMATION_FLIP && abs(_player->position.x - position.x) > 64)
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
D2D1_RECT_F Katherine::GetRect()
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
pair<D2D1_RECT_F, int8_t> Katherine::GetAttackRect()
{
	D2D1_RECT_F rc = {};

	if (_ani == ANIMATION_STRIKE1)
	{
		rc.top = 50;
		rc.bottom = 70;

		if (_forward)
		{
			rc.left = 182;
			rc.right = 202;
		}
		else
		{
			rc.left = -148;
			rc.right = -128;
		}
	}
	else if (_ani == ANIMATION_STRIKE2)
	{
		rc.top = 10;
		rc.bottom = 40;

		if (_forward)
		{
			rc.left = 50;
			rc.right = 75;
		}
		else
		{
			rc.left = -25;
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

void Katherine::stopFalling(float collisionSize)
{
	_speed.y = 0;
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
	_speed.x = ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_forward = true;
}
void Katherine::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	_speed.x = -ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_forward = false;
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
void Katherine::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);
		if (deltaX < 48 && deltaY < 16) // CC is close to K
		{
			_ani = ANIMATION_STRIKE2;
			_ani->reset();
			_isAttack = true;
			_forward = _player->position.x > position.x;

			_attackRest = 800;
		}
		else if (192 < deltaX && deltaX < 208 && deltaY < 24) // CC is far from K
		{
			_ani = ANIMATION_STRIKE1;
			_ani->reset();
			_isAttack = true;
			_forward = _player->position.x > position.x;

			_attackRest = 1200;
		}
	}
}
bool Katherine::checkForHurts()
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
			// jump/block every 4 hits
			_hitsCuonter = (_hitsCuonter + 1) % 4;
			_canJump = true;
			return true;
		}
	}

	return false;
}
