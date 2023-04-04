#include "HermitCrab.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_SHOOT		_animations.at(_shootAniName)
#define ANIMATION_IDLE		_animations.at(_idleAniName)


HermitCrab::HermitCrab(const WwdObject& obj, Player* player, bool isFromNest)
	: BaseEnemy(obj, player, 1, 10, "FASTADVANCE", "HIT", "HIT", "KILLFALL",
		"STRIKE1", "", "STRIKE1", "", ""), _isFromNest(isFromNest)
{
	if (isFromNest)
	{
		_speed.x = obj.speedX / 1000.f;
		_speed.y = obj.speedY / 1000.f;
		_isMirrored = obj.speedX > 0;
	}
}

void HermitCrab::Logic(uint32_t elapsedTime)
{
	if (_isFromNest)
	{
		_speed.y += GRAVITY * elapsedTime;
		position.y += _speed.y * elapsedTime;
		position.x += _speed.x * elapsedTime;
		PostLogic(elapsedTime);
	}
	else
	{
		BaseEnemy::Logic(elapsedTime);
	}
}

void HermitCrab::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x);
		const float deltaY = abs(_player->position.y - position.y);

		if (deltaX < 256 && deltaY < 42)
		{
			_ani = ANIMATION_SHOOT;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
			obj.y = (int32_t)position.y - 24;
			obj.z = ZCoord;
			obj.speedX = _forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
			obj.damage = 10;
			ActionPlane::addPlaneObject(DBG_NEW CrabBomb(obj));

			_attackRest = 1200;
		}
	}
}

void HermitCrab::stopFalling(float collisionSize)
{
	_speed = {};
	position.y -= collisionSize;
	_isFromNest = false;
	_speed.x = ENEMY_PATROL_SPEED;

	// TODO: fix movement range (for all enemies and especially for the carbs)
	myMemCpy(_minX, position.x - 32);
	myMemCpy(_maxX, position.x + 32);
}

pair<D2D1_RECT_F, int8_t> HermitCrab::GetAttackRect() { return {}; }
bool HermitCrab::isDuck() const { return true; }
bool HermitCrab::isTakeDamage() const { return false; }
