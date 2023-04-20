#include "Wolvington.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_STRIKE1	_animations.at("STRIKE1") // sword down
#define ANIMATION_STRIKE2	_animations.at("STRIKE2") // hand up (boxings)
#define ANIMATION_STRIKE3	_animations.at("STRIKE3") // magic up
#define ANIMATION_STRIKE4	_animations.at("STRIKE4") // magic down
#define ANIMATION_BLOCK		_animations.at("BLOCK")
#define ANIMATION_JUMPBACK	_animations.at("JUMP")


Wolvington::Wolvington(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "", "", ""),
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

		_speed.y = -(0.64f - 75 * GRAVITY); // see LeRauex ...
		_speed.x = (position.x < _player->position.x) ? -0.35f : 0.35f;

		_isAttack = false;
		_canJump = false;
	}

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
		_forward = _speed.x > 0;
	}

	if (_ani != ANIMATION_JUMPBACK && abs(_player->position.x - position.x) > 64)
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
Rectangle2D Wolvington::GetRect()
{
	Rectangle2D rc;

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
pair<Rectangle2D, int> Wolvington::GetAttackRect()
{
	Rectangle2D rc;

	if (_ani == ANIMATION_STRIKE1 || _ani == ANIMATION_STRIKE2)
	{
		if (_forward)
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

		// set rectangle by center
		const float addX = position.x - (rc.right - rc.left) / 2, addY = position.y - (rc.bottom - rc.top) / 2;
		rc.top += addY;
		rc.bottom += addY;
		rc.left += addX;
		rc.right += addX;
	}
	else return {};

	// strikes 3,4 are "magic attack" so they have no rectangle

	return { rc, _damage };
}

void Wolvington::stopFalling(float collisionSize)
{
	_speed.y = 0;
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
	_speed.x = ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_forward = true;
}
void Wolvington::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	_speed.x = -ENEMY_PATROL_SPEED;
	_speed.y = 0;
	_forward = false;
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
			position.x += _speed.x * elapsedTime;
			position.y += _speed.y * elapsedTime;
			_speed.y += GRAVITY * elapsedTime;

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
void Wolvington::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x);
		if (deltaX < 64) // CC is close to W
		{
			if (_player->isDuck()) _ani = ANIMATION_STRIKE1;
			else _ani = ANIMATION_STRIKE2;
			_ani->reset();
			_isAttack = true;
			_forward = _player->position.x > position.x;

			_attackRest = 700;

			_magicAttackCuonter = 0;
		}
		else if (deltaX < 256) // CC is far from W
		{
			if (_magicAttackCuonter < 3)
			{
				WwdObject obj;
				obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
				obj.y = (int32_t)position.y - 20;
				obj.z = ZCoord;
				obj.speedX = _forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
				obj.damage = 20;

				if (_player->isDuck()) {
					_ani = ANIMATION_STRIKE4;
					obj.y += 30;
				}
				else
					_ani = ANIMATION_STRIKE3;
				
				_ani->reset();
				_isAttack = true;
				_forward = _player->position.x > position.x;

				ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(obj, "LEVEL6/IMAGES/WOLVINGTONMAGIC"));

				_attackRest = 1500;

				_magicAttackCuonter += 1;
			}
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

	if (checkForHurt(_player->GetAttackRect()))
	{
		// jump every 2 hits
		_hitsCuonter = (_hitsCuonter + 1) % 2;
		_canJump = true;
		_magicAttackCuonter = 0;
		return true;
	}

	return false;
}
