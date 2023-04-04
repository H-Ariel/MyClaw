#include "Siren.h"
#include "../Player.h"
#include "../ActionPlane.h"


Siren::Siren(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 10, 0, "FASTADVANCE", "HITHIGH",
		"HITLOW", "KILLFALL", "", "", "", "", "")
{
}

D2D1_RECT_F Siren::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 50;
	_saveCurrRect.bottom = position.y + 40;
	return _saveCurrRect;
}

pair<D2D1_RECT_F, int8_t> Siren::GetAttackRect()
{
	return {};
}

void Siren::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		if (abs(_player->position.x - position.x) < 96 && abs(_player->position.y - position.y) < 24)
		{
			_ani = _animations["STRIKE1"];
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
			obj.y = (int32_t)position.y;
			obj.z = ZCoord;
			obj.speedX = _forward ? 150 : -150;
			ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj));
		}
	}
}
