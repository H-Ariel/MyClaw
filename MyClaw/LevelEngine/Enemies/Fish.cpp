#include "Fish.h"
#include "../GlobalObjects.h"


#define ANIMATION_JUMP		_animations.at("JUMP")
#define ANIMATION_STRIKE	_animations.at("STRIKE1")


Fish::Fish(const WwdObject& obj)
	: BaseEnemy(obj, 1, 10, "SWIM", "HIT", "HIT",
		"KILLFALL", "", "", "", "", "", ENEMY_PATROL_SPEED, true), _yPos((float)obj.y)
{
}

void Fish::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_ani == ANIMATION_JUMP)
	{
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
		if (_ani->isFinishAnimation())
			_ani = _aniWalk;
	}
	else
		position.y = _yPos;

	position.x += speed.x * elapsedTime;
	if (_isAttack) // duoble speed when it attack
		position.x += speed.x * elapsedTime;
	if (position.x < _minX) stopMovingLeft(_minX - position.x);
	else if (position.x > _maxX) stopMovingRight(position.x - _maxX);

	if (!_isAttack)
	{
		if (_attackTimer.isFinished()) // TODO search when i update this timer :/
			BaseEnemy::makeAttack();
	}
	else
	{
		if (_ani->isFinishAnimation())
		{
			_ani = _aniWalk;
			_ani->reset();
			_isAttack = false;
			_isMirrored = speed.x < 0;
		}
	}

	PostLogic(elapsedTime);
}

pair<Rectangle2D, int> Fish::GetAttackRect()
{
	return { _saveCurrRect, _damage };
}

void Fish::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;
	speed.x = ENEMY_PATROL_SPEED;
	_isMirrored = false;
}
void Fish::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	speed.x = -ENEMY_PATROL_SPEED;
	_isMirrored = true;
}

void Fish::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 96 && deltaY < 32)
	{
		_ani = ANIMATION_STRIKE;

		// TODO: jump if CC on land next to water
	//	_ani = ANIMATION_JUMP;
	//	speed.y = -0.6f;
		_ani->reset();
		_isStanding = false;
		_isAttack = true;
		_isMirrored = GO::getPlayerPosition().x < position.x;
	}
}
