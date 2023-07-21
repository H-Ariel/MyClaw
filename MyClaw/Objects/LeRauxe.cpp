#include "LeRauxe.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_STRIKE	_animations.at("STRIKE")
#define ANIMATION_STAB		_animations.at("STAB")
#define ANIMATION_BLOCKHIGH	_animations.at("BLOCKHIGH")
#define ANIMATION_BLOCKLOW	_animations.at("BLOCKLOW")
#define ANIMATION_JUMPBACK	_animations.at("JUMPBACK")


LeRauxe::LeRauxe(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "ADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "", "", "")
{
}

void LeRauxe::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;


	const shared_ptr<const Animation> prevAni = _ani;

	if (_hitsCuonter == 0 && _canJump)
	{
		_ani = ANIMATION_JUMPBACK;

		/*
		y = 0.5 * a * t^2 + v0 * t + y0
		
		v0 = ?
		a = GRAVITY
		y0 = 0
		y = 96
		t = 150
		
		96 = 0.5 * GRAVITY * 22500 + v0 * 150 + 0
		...
		*/
		_speed.y = -(0.64f - 75 * GRAVITY);
		_speed.x = (position.x < _player->position.x) ? -0.35f : 0.35f;

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
	_speed.y += GRAVITY * elapsedTime;
	position.y += _speed.y * elapsedTime;

	if (!_isAttack && _attackRest <= 0 && _ani != ANIMATION_JUMPBACK)
	{
		makeAttack();
	}
	else if (_ani->isFinishAnimation())
	{
		_ani = ANIMATION_WALK;
		_isAttack = false;
		_isMirrored = _speed.x < 0;
	}
	
	if (_ani != ANIMATION_JUMPBACK && abs(_player->position.x - position.x) > 64)
	{
		_isMirrored = _player->position.x < position.x;
		if (!_isMirrored) _speed.x = abs(_speed.x);
		else _speed.x = -abs(_speed.x);
	}

	if (_ani != prevAni)
	{
		_ani->reset();
	}


	PostLogic(elapsedTime);
}
Rectangle2D LeRauxe::GetRect()
{
	_saveCurrRect.left = -7.f + 15 * _isMirrored;
	_saveCurrRect.right = _saveCurrRect.left + 50;
	_saveCurrRect.top = 5;
	_saveCurrRect.bottom = 115;

	_saveCurrRect = setRectByCenter(_saveCurrRect);
	return _saveCurrRect;
}
pair<Rectangle2D, int> LeRauxe::GetAttackRect()
{
	Rectangle2D rc;

	if (_ani == ANIMATION_STRIKE)
	{
		rc.top = 60;
		rc.bottom = 80;

		if (_isMirrored)
		{
			rc.left = -80;
			rc.right = 0;
		}
		else
		{
			rc.left = 50;
			rc.right = 130;
		}
	}
	else if (_ani == ANIMATION_STAB)
	{
		rc.top = 30;
		rc.bottom = 50;

		if (_isMirrored)
		{
			rc.left = -80;
			rc.right = 0;
		}
		else
		{
			rc.left = 50;
			rc.right = 130;
		}
	}
	else return {};

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}

void LeRauxe::stopFalling(float collisionSize)
{
	_speed.y = 0;
	position.y -= collisionSize;
	
	if (_ani == ANIMATION_JUMPBACK)
	{
		stopMovingLeft(0); // TODO: maybe use `stopMovingRight`? or both?
		_ani = ANIMATION_WALK;
	}
}
void LeRauxe::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;
	_speed.x = ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_isMirrored = false;
}
void LeRauxe::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	_speed.x = -ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_isMirrored = true;
}

bool LeRauxe::PreLogic(uint32_t elapsedTime)
{
	if (_attackRest > 0)
	{
		_attackRest -= elapsedTime;
	}

	if (_ani == ANIMATION_JUMPBACK)
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
	if (enemySeeClaw())
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);
		if (deltaX < 96 && deltaY < 16) // CC is close to LR
		{
			if (_player->isDuck()) _ani = ANIMATION_STRIKE;
			else _ani = ANIMATION_STAB;
			_ani->reset();
			_isAttack = true;
			_isMirrored = _player->position.x < position.x;

			_attackRest = 600;
		}
	}
}
bool LeRauxe::checkForHurts()
{
	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isClawProjectile(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
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
