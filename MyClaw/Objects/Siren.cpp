#include "Siren.h"
#include "../Player.h"
#include "../ActionPlane.h"


Siren::Siren(const WwdObject& obj)
	: BaseEnemy(obj, 10, 0, "FASTADVANCE", "HITHIGH",
		"HITLOW", "KILLFALL", "", "", "", "", "", 0.1f)
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

pair<Rectangle2D, int> Siren::GetAttackRect()
{
	return {};
}

void Siren::makeAttack()
{
	// that ugly... but works fine :)
	// TODO: rewrite this (?)
	if (!player->isFreeze() && _isStanding && enemySeeClaw())
	{
		if (abs(player->position.x - position.x) < 128 && abs(player->position.y - position.y) < 24)
		{
			_ani = _animations["STRIKE1"];
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (_isMirrored ? _saveCurrRect.left - _saveCurrRect.right : _saveCurrRect.right - _saveCurrRect.left));
			obj.y = (int32_t)position.y;
			obj.z = ZCoord;
			obj.speedX = _isMirrored ? -100 : 100;
			ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 0));
			ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 250));
			ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 500));
			ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 750));
			ActionPlane::addPlaneObject(DBG_NEW SirenProjectile(obj, 1000));

			_attackRest = 250;
		}
	}
}
