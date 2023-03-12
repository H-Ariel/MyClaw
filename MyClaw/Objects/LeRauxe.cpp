#include "LeRauxe.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_STRIKE	_animations.at("STRIKE")
#define ANIMATION_STAB		_animations.at("STAB")
#define ANIMATION_HITHIGH	_animations.at(_hit1AniName)
#define ANIMATION_HITLOW	_animations.at(_hit2AniName)
#define ANIMATION_BLOCKHIGH	_animations.at("BLOCKHIGH")
#define ANIMATION_BLOCKLOW	_animations.at("BLOCKLOW")
#define ANIMATION_JUMP		_animations.at("JUMP")


// TODO: same to "Seagull" ...
inline float calcSpeed(float srcPos, float dstPos, int msTime = 500)
{
	return (dstPos - srcPos) / msTime;
}


LeRauxe::LeRauxe(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 100, 10, "ADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE", "", "", { { "HOME", 1000 }, { "HOME1", 1000 } }),
	_attackRest(0), _hitsCuonter(0)
{
}

void LeRauxe::Logic(uint32_t elapsedTime)
{
	// Pre Logic
	if (!PreLogic(elapsedTime)) return;

	const shared_ptr<Animation> prevAni = _ani;

	if (_ani == ANIMATION_BLOCKHIGH || _ani == ANIMATION_BLOCKLOW)
	{
		if (_ani->isFinishAnimation())
		{
			_isStanding = true;
		}
		else
		{
			PostLogic(elapsedTime);
			return;
		}
	}

	// Real Logic
	
	if (_hitsCuonter >= /*3*/1)
	{
		static const int msTime = 500/2;
		_hitsCuonter = 0;
		_ani = ANIMATION_JUMP;
		_speed.x = calcSpeed(position.x, position.x > _player->position.x ? _minX : _maxX, msTime*2);
		_speed.y = -(192.f / msTime - (GRAVITY * msTime) / 2);
		_isStanding = false;
		_isAttack = false;
	}

	if (_isStanding)
	{
		_ani = _standAni[_standAniIdx]->ani;

		if (_standAni[_standAniIdx]->elapsedTime >= _standAni[_standAniIdx]->duration)
		{
			_standAni[_standAniIdx]->elapsedTime = 0;
			_standAni[_standAniIdx]->ani->reset();

			_standAniIdx += 1;

			if (_standAniIdx == _standAni.size())
			{
				if (!_isStaticEnemy)
				{
					_isStanding = false;
					_ani = ANIMATION_WALK;
				}
				_standAniIdx = 0;
			}
		}
		else
		{
			_standAni[_standAniIdx]->elapsedTime += elapsedTime;
		}
	}

	if (!_isStanding && !_isAttack && !_isStaticEnemy)
	{
		position.x += _speed.x * elapsedTime;
		position.y += _speed.y * elapsedTime;
		if (_ani == ANIMATION_JUMP)
			_speed.y += GRAVITY * elapsedTime;

		if (position.x < _minX) { stopMovingLeft(_minX - position.x); position.y = _player->position.y; }
		else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); position.y = _player->position.y; }
	}

	if (_attackRest > 0)
	{
		_attackRest -= elapsedTime;
	}

	if (!_isAttack && _attackRest <= 0 && _ani != ANIMATION_JUMP)
	{
		if (_isStaticEnemy)
		{
			_isStanding = true;
		}
		makeAttack();
	}
	else
	{
		if (_ani->isFinishAnimation())
		{
			_ani = ANIMATION_WALK;
			_isAttack = false;
			_forward = _speed.x > 0;
		}
	}

	// TODO: sometime LR blocks CC sword attack, so we need to continue this case
	// TODO: sometime LR jump, so we need to continue this case


	if (_ani != prevAni)
		_ani->reset();

	// Post Logic
	PostLogic(elapsedTime);
}

void LeRauxe::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);
		if (deltaX < 96 && deltaY < 16) // CC is close to enemy
		{
			if (_canStrike)
			{
			//	_ani = getRandomInt(0, 1) == 0 ? ANIMATION_STRIKE : ANIMATION_STAB;
				if (_player->isDuck()) _ani = ANIMATION_STRIKE;
				else _ani = ANIMATION_STAB;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;

				_attackRest = 500;
			}
		}
	}
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

bool LeRauxe::checkForHurts()
{
	if (_ani != ANIMATION_JUMP)
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
			_hitsCuonter += 1;
			return true;
		}
	}

	return false;
}
