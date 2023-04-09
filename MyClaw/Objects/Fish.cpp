#include "Fish.h"
#include "../Player.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_JUMP		_animations.at("JUMP")
#define ANIMATION_STRIKE	_animations.at("STRIKE1")


Fish::Fish(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 1, 10, "SWIM", "HIT", "HIT",
		"KILLFALL", "", "", "", "", "", 0.1f, true), _yPos((float)obj.y)
{
}

void Fish::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_ani == ANIMATION_JUMP)
	{
		_speed.y += GRAVITY * elapsedTime;
		position.y += _speed.y * elapsedTime;
		if (_ani->isFinishAnimation())
			_ani = ANIMATION_WALK;
	}
	else
		position.y = _yPos;

	position.x += _speed.x * elapsedTime;
	if (_isAttack) // duoble speed when it attack
		position.x += _speed.x * elapsedTime;
	if (position.x < _minX) stopMovingLeft(_minX - position.x);
	else if (position.x > _maxX) stopMovingRight(position.x - _maxX);

	if (!_isAttack)
	{
		if (_attackRest > 0)
			_attackRest -= elapsedTime;
		if (_attackRest <= 0)
			makeAttack();
	}
	else
	{
		if (_ani->isFinishAnimation())
		{
			_ani = ANIMATION_WALK;
			_ani->reset();
			_isAttack = false;
			_forward = _speed.x > 0;
		}
	}

	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> Fish::GetAttackRect()
{
	return { _saveCurrRect, _damage };
}

void Fish::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;
	_speed.x = ENEMY_PATROL_SPEED;
	_forward = true;
}
void Fish::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	_speed.x = -ENEMY_PATROL_SPEED;
	_forward = false;
}

void Fish::makeAttack()
{
	if ((_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x))
	{
		if (abs(_player->position.x - position.x) < 96 && abs(_player->position.y - position.y) < 32)
		{
			_ani = ANIMATION_STRIKE;

			// TODO: jump if CC on land next to water
		//	_ani = ANIMATION_JUMP;
		//	_speed.y = -0.6f;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;
		}
	}
}
