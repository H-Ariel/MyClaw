#include "Seagull.h"
#include "../Player.h"


inline float calcSpeed(float srcPos, float dstPos, int msTime = 500)
{
	return (dstPos - srcPos) / msTime;
}


Seagull::Seagull(const WwdObject& obj)
	: BaseEnemy(obj, 1, 10, "FLYING", "FEATHEREXPLODE",
		"FEATHERFLOAT", "KILLFALL", "", "", "", "", "", ENEMY_PATROL_SPEED),
	_state(States::Fly), _minY((float)obj.y), _maxY((float)obj.y + 192)
{
	// TODO: some of them has no treasure
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
		if (_attackRest <= 0 && enemySeeClaw() &&
			(_minX <= player->position.x && player->position.x <= _maxX) &&
			(_minY <= player->position.y && player->position.y <= _maxY))
		{
			_state = States::DiveIn;
			_isAttack = true;
			_speed.x = calcSpeed(position.x, player->position.x);
			_speed.y = calcSpeed(position.y, player->position.y);
			_ani = _animations["DIVE" + to_string(getRandomInt(1, 4))];
		}
		break;

	case States::DiveIn:
		if (position.y >= player->position.y || GetRect().intersects(player->GetRect()))
		{
			_state = States::DiveOut;
			_isAttack = false;
			_speed.x = calcSpeed(position.x, _isMirrored ? _minX : _maxX); // TODO: find better `dstPos`
			_speed.y = calcSpeed(position.y, _minY);

			_ani = _animations["HOME"];
		}
		break;

	case States::DiveOut:
		if (position.y < _minY)
		{
			position.y = _minY;
			_speed.x = _isMirrored ? ENEMY_PATROL_SPEED : -ENEMY_PATROL_SPEED;
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

pair<Rectangle2D, int> Seagull::GetAttackRect()
{
	if (_state == States::Fly) return {};
	return { _saveCurrRect, _damage };
}
