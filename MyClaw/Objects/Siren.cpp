#include "Siren.h"
#include "../Player.h"
#include "../ActionPlane.h"


Siren::Siren(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 10, 0, "FASTADVANCE", "HITHIGH",
		"HITLOW", "KILLFALL", "", "", "", "", "")
{
}

Rectangle2D Siren::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 50;
	_saveCurrRect.bottom = position.y + 40;
	return _saveCurrRect;
}

pair<Rectangle2D, int8_t> Siren::GetAttackRect()
{
	return {};
}

void Siren::makeAttack()
{
	// that ugly... but works fine :)
	// TODO: rewrite this (?)
	if (!_player->isFreeze() && _isStanding)
	{
		if ((_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x))
		{
			if (abs(_player->position.x - position.x) < 128 && abs(_player->position.y - position.y) < 24)
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
				obj.speedX = _forward ? 100 : -100;
				ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 0));
				ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 250));
				ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 500));
				ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 750));
				ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 1000));

				_attackRest = 250;
			}
		}
	}
}
