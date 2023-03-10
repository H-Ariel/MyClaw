#include "Seagull.h"
#include "../Player.h"

// TODO : continue speed and times

#define DIVE_SPEED_X	(abs(position.x - _player->position.x) / 500.f)
#define DIVE_SPEED_Y	((_maxY - _minY) / 500.f)


Seagull::Seagull(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 1, 10, "FLYING", "FEATHEREXPLODE", "FEATHERFLOAT", "KILLFALL", "", "", "", "", {}),
	_state(States::Fly), _minY(obj.y), _maxY(obj.y + 192)
{
}

void Seagull::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	position.x += _speed.x * elapsedTime;
	position.y += _speed.y * elapsedTime;
	if (position.x < _minX) { stopMovingLeft(_minX - position.x); }
	else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); }

	switch (_state)
	{
	case States::Fly: {
		const bool canDive = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

		if (canDive &&
			(_minX <= _player->position.x && _player->position.x <= _maxX) &&
			(_minY <= _player->position.y && _player->position.y <= _maxY)
			) // if CC close to enemy - dive
		{
			_state = States::DiveIn;
			_isAttack = true;
			_speed.x = _forward ? DIVE_SPEED_X : -DIVE_SPEED_X;
			_speed.y = DIVE_SPEED_Y;
			_ani = _animations["DIVE1"];
		}
	}	break;

	case States::DiveIn:
		if (
			(position.y > _player->position.y) ||
			CollisionDistances::isCollision(GetRect(), _player->GetRect())
			)
		{
			_state = States::DiveOut;
			_isAttack = false;
			_speed.x = _forward ? -DIVE_SPEED_X : DIVE_SPEED_X;
			_speed.y = -DIVE_SPEED_Y;

			_ani = _animations["DIVE2"];
		}
		break;

	case States::DiveOut:

		if (position.y < _minY)
		{
			position.y = _minY;
			_speed.x = _forward ? -ENEMY_PATROL_SPEED : ENEMY_PATROL_SPEED;
			_speed.y = 0;

			_ani = _animations["FLYING"];
			_state = States::Fly;
		}

		break;
	}

	PostLogic(elapsedTime);

	_saveCurrRect = _ani->GetRect();
	_isAttack = _state == States::DiveIn;
}

D2D1_RECT_F Seagull::GetRect()
{
	return _saveCurrRect;
}

pair<D2D1_RECT_F, int8_t> Seagull::GetAttackRect()
{
	if (_state == States::Fly) return {};
	return { _saveCurrRect, _damage };
}
