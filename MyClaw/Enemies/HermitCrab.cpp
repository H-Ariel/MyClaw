#include "HermitCrab.h"
#include "../Player.h"
#include "../ActionPlane.h"
#include "../Objects/EnemyProjectile.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_SHOOT		_animations.at(_shootAniName)
#define ANIMATION_IDLE		_animations.at(_idleAniName)


HermitCrab::HermitCrab(const WwdObject& obj, bool isFromNest)
	: BaseEnemy(obj, 1, 10, "FASTADVANCE", "HIT", "HIT", "KILLFALL",
		"STRIKE1", "", "STRIKE1", "", "", ENEMY_PATROL_SPEED), _isFromNest(isFromNest)
{
	if (isFromNest)
	{
		speed.x = obj.speedX / 1000.f;
		speed.y = obj.speedY / 1000.f;
		_isMirrored = obj.speedX > 0;
	}

	// TODO: find black carbs (maybe `if (obj.smarts == 1)`)
	// TODO: throw bombs only `if (obj.userValue1 == 1)`
}

void HermitCrab::Logic(uint32_t elapsedTime)
{
	if (_isFromNest)
	{
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
		position.x += speed.x * elapsedTime;
		PostLogic(elapsedTime);
	}
	else
	{
		BaseEnemy::Logic(elapsedTime);
	}
}

void HermitCrab::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 256 && deltaY < 42)
	{
		_ani = ANIMATION_SHOOT;
		_ani->reset();
		_isStanding = false;
		_isAttack = true;
		_isMirrored = player->position.x < position.x;

		WwdObject obj;
		obj.x = (int32_t)(position.x + (_isMirrored ? _saveCurrRect.left - _saveCurrRect.right : _saveCurrRect.right - _saveCurrRect.left));
		obj.y = (int32_t)position.y - 24;
		obj.z = ZCoord;
		obj.speedX = _isMirrored ? -DEFAULT_PROJECTILE_SPEED : DEFAULT_PROJECTILE_SPEED;
		obj.damage = 10;
		ActionPlane::addPlaneObject(DBG_NEW CrabBomb(obj));

		_attackRest = 1200;
	}
}

void HermitCrab::stopFalling(float collisionSize)
{
	if (_isFromNest)
	{
		// find enemy range (copied from BaseEnemy)
		auto range = ActionPlane::getPhysicsManager().getEnemyRange(position, _minX, _maxX);
		myMemCpy(_minX, range.first);
		myMemCpy(_maxX, range.second);
		speed.x = ENEMY_PATROL_SPEED;
		_isFromNest = false;
	}
	else
	{
		BaseEnemy::stopFalling(collisionSize);
	}
}

pair<Rectangle2D, int> HermitCrab::GetAttackRect() { return {}; }
bool HermitCrab::isDuck() const { return true; }
bool HermitCrab::isTakeDamage() const { return false; }
