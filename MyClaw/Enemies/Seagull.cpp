#include "Seagull.h"
#include "../Objects/Player.h"


#define ANIMATION_FLYING	_animations["FLYING"]
#define ANIMATION_DIVE		_animations["DIVE"]
#define ANIMATION_RETURN	_animations["HOME"]


Seagull::Seagull(const WwdObject& obj)
	: BaseEnemy(obj, 1, 10, "FLYING", "FEATHEREXPLODE",
		"FEATHERFLOAT", "KILLFALL", "", "", "", "", "", ENEMY_PATROL_SPEED),
	_minY((float)obj.y), _maxY((float)obj.y + 160)
{
	// some of them has no treasure
	if (obj.powerup == 0 && obj.userRect1.left == 0 && obj.userRect1.right == 0 &&
		obj.userRect1.bottom == 0 && obj.userRect1.top == 0 && obj.userRect2.left == 0 &&
		obj.userRect2.right == 0 && obj.userRect2.bottom == 0 && obj.userRect2.top == 0)
	{
		_itemsTypes.clear();
	}
}

void Seagull::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_ani == ANIMATION_FLYING)
	{
		_attackRest -= elapsedTime;

		// if CC close to enemy - dive and attack
		if (_attackRest <= 0 && doesEnemySeeClaw() &&
			(_minX <= player->position.x && player->position.x <= _maxX) &&
			(_minY <= player->position.y && player->position.y <= _maxY))
		{
			_isAttack = true;
			speed.x = _isMirrored ? -0.3f : 0.3f;
			speed.y = 0.5f;
			_ani = ANIMATION_DIVE;
		}
	}
	else if (_ani == ANIMATION_DIVE)
	{
		if (position.y >= player->position.y)
		{
			_isAttack = false;
			speed.y = -speed.y;
			_ani = ANIMATION_RETURN;
		}
	}
	else // if (_ani == ANIMATION_RETURN)
	{
		if (position.y < _minY)
		{
			position.y = _minY;
			speed.x = _isMirrored ? ENEMY_PATROL_SPEED : -ENEMY_PATROL_SPEED;
			speed.y = 0;
			_attackRest = 1000;
			_ani = ANIMATION_FLYING;
		}
	}

	position.x += speed.x * elapsedTime;
	position.y += speed.y * elapsedTime;

	if (_ani == ANIMATION_FLYING)
	{
		if (position.x < _minX) { stopMovingLeft(_minX - position.x); }
		else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); }
		_isMirrored = speed.x < 0;
	}

	PostLogic(elapsedTime);
}

pair<Rectangle2D, int> Seagull::GetAttackRect()
{
	if (_ani == ANIMATION_FLYING) return {};
	return { _saveCurrRect, _damage };
}
