#include "Wolvington.h"
#include "../Player.h"
#include "../ActionPlane.h"
#include "../Objects/EnemyProjectile.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_STRIKE1	_animations.at("STRIKE1") // sword down
#define ANIMATION_STRIKE2	_animations.at("STRIKE2") // hand up (boxings)
#define ANIMATION_STRIKE3	_animations.at("STRIKE3") // magic up
#define ANIMATION_STRIKE4	_animations.at("STRIKE4") // magic down
#define ANIMATION_BLOCK		_animations.at("BLOCK")
#define ANIMATION_JUMPBACK	_animations.at("JUMP")


Wolvington::Wolvington(const WwdObject& obj)
	: BaseBoss(obj, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "", "", ""),
	_magicAttackCuonter(0)
{
}

void Wolvington::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	
	const shared_ptr<Animation> prevAni = _ani;

	if (_hitsCuonter == 0 && _canJump)
	{
		_ani = ANIMATION_JUMPBACK;

		speed.y = -(0.64f - 75 * GRAVITY); // see LeRauex ...
		speed.x = (position.x < player->position.x) ? -0.35f : 0.35f;

		_isAttack = false;
		_canJump = false;
	}

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

	if (!_isAttack && _attackRest <= 0 && _ani != ANIMATION_JUMPBACK)
	{
		BaseBoss::makeAttack();
	}
	else if (_ani->isFinishAnimation())
	{
		_ani = ANIMATION_WALK;
		_isAttack = false;
		_isMirrored = speed.x < 0;
	}

	if (_ani != ANIMATION_JUMPBACK && abs(player->position.x - position.x) > 64)
	{
		_isMirrored = player->position.x < position.x;
		if (!_isMirrored) speed.x = abs(speed.x);
		else speed.x = -abs(speed.x);
	}

	if (_ani != prevAni)
	{
		_ani->reset();
	}


	PostLogic(elapsedTime);
}
Rectangle2D Wolvington::GetRect()
{
	_saveCurrRect.left = -7.f + 15 * (_isMirrored);
	_saveCurrRect.right = _saveCurrRect.left + 50;
	_saveCurrRect.top = 5;
	_saveCurrRect.bottom = 115;
	_saveCurrRect = setRectByCenter(_saveCurrRect);
	return _saveCurrRect;
}
pair<Rectangle2D, int> Wolvington::GetAttackRect()
{
	Rectangle2D rc;

	if (_ani == ANIMATION_STRIKE1 || _ani == ANIMATION_STRIKE2)
	{
		if (!_isMirrored)
		{
			rc.left = 40;
			rc.right = 70;
		}
		else
		{
			rc.left = -40;
			rc.right = -70;
		}

		rc.top = 40;
		rc.bottom = 60;

		if (_ani == ANIMATION_STRIKE2)
		{
			rc.top -= 60;
			rc.bottom -= 60;
		}

		rc = setRectByCenter(rc);
	}
	else return {};

	// strikes 3,4 are "magic attack" so they have no rectangle

	return { rc, _damage };
}

void Wolvington::stopFalling(float collisionSize)
{
	speed.y = 0;
	position.y -= collisionSize;

	if (_ani == ANIMATION_JUMPBACK)
	{
		stopMovingLeft(0);
		_ani = ANIMATION_WALK;
	}
}
void Wolvington::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;
	speed.x = ENEMY_PATROL_SPEED;
	speed.y = 0;
	_isMirrored = false;
}
void Wolvington::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	speed.x = -ENEMY_PATROL_SPEED;
	speed.y = 0;
	_isMirrored = true;
}

bool Wolvington::PreLogic(uint32_t elapsedTime)
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
			position.x += speed.x * elapsedTime;
			speed.y += GRAVITY * elapsedTime;
			position.y += speed.y * elapsedTime;

			PostLogic(elapsedTime);
			return false;
		}
	}

	if (!BaseBoss::PreLogic(elapsedTime))
		return false;

	if (_ani == ANIMATION_BLOCK || _isAttack)
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
void Wolvington::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 64) // CC is close to W
	{
		if (player->isDuck()) _ani = ANIMATION_STRIKE1;
		else _ani = ANIMATION_STRIKE2;
		_ani->reset();
		_isAttack = true;
		_isMirrored = player->position.x < position.x;

		_attackRest = 700;

		_magicAttackCuonter = 0;
	}
	else if (deltaX < 256) // CC is far from W
	{
		if (_magicAttackCuonter < 3)
		{
			WwdObject obj;
			obj.x = (int32_t)(position.x + (!_isMirrored ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
			obj.y = (int32_t)position.y - 20;
			obj.speedX = (!_isMirrored) ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
			obj.damage = 20;

			if (player->isDuck()) {
				_ani = ANIMATION_STRIKE4;
				obj.y += 30;
			}
			else
				_ani = ANIMATION_STRIKE3;

			_ani->reset();
			_isAttack = true;
			_isMirrored = player->position.x < position.x;

			ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(obj, "LEVEL_WOLVINGTONMAGIC"));

			_attackRest = 1500;

			_magicAttackCuonter += 1;
		}
	}
}
bool Wolvington::checkForHurts()
{
	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isClawProjectile(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				_ani = ANIMATION_BLOCK;
				_ani->reset();
				_magicAttackCuonter = 0;
				return false;
			}
		}
	}

	if (checkClawHit())
	{
		// jump every 2 hits
		_hitsCuonter = (_hitsCuonter + 1) % 2;
		_canJump = true;
		_magicAttackCuonter = 0;
		return true;
	}

	return false;
}
