#include "Seagull.h"
#include "../Player.h"


inline float calcSpeed(float srcPos, float dstPos, int msTime = 500)
{
	return (dstPos - srcPos) / msTime;
}


Seagull::Seagull(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 1, 10, "FLYING", "FEATHEREXPLODE",
		"FEATHERFLOAT", "KILLFALL", "", "", "", "", ""),
	_state(States::Fly), _minY((float)obj.y), _maxY((float)obj.y + 192)
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
	case States::Fly:
		_attackRest -= elapsedTime;

		// if CC close to enemy - dive
		if (_attackRest <= 0 &&
			((_forward && _player->position.x > position.x) ||
				(!_forward && _player->position.x < position.x)) &&
			(_minX <= _player->position.x && _player->position.x <= _maxX) &&
			(_minY <= _player->position.y && _player->position.y <= _maxY))
		{
			_state = States::DiveIn;
			_isAttack = true;
			_speed.x = calcSpeed(position.x, _player->position.x);
			_speed.y = calcSpeed(position.y, _player->position.y);
			_ani = _animations["DIVE" + to_string(getRandomInt(1, 4))];
		}
		break;

	case States::DiveIn:
		if (position.y >= _player->position.y || GetRect().intersects(_player->GetRect()))
		{
			_state = States::DiveOut;
			_isAttack = false;
			_speed.x = calcSpeed(position.x, _forward ? _maxX : _minX); // TODO: find better `dstPos`
			_speed.y = calcSpeed(position.y, _minY);

			_ani = _animations["HOME"];
		}
		break;

	case States::DiveOut:
		if (position.y < _minY)
		{
			position.y = _minY;
			_speed.x = _forward ? -ENEMY_PATROL_SPEED : ENEMY_PATROL_SPEED;
			_speed.y = 0;
			_attackRest = 1000;

			_ani = _animations["FLYING"];
			_state = States::Fly;
		}
		break;
	}

	PostLogic(elapsedTime);

	_isAttack = _state == States::DiveIn;
}

pair<Rectangle2D, int8_t> Seagull::GetAttackRect()
{
	if (_state == States::Fly) return {};
	return { _saveCurrRect, _damage };
}
