#include "Siren.h"
#include "../ActionPlane.h"
#include "../Objects/EnemyProjectile.h"


Siren::Siren(const WwdObject& obj)
	: BaseEnemy(obj, 10, 0, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "", "", "", "", "", ENEMY_PATROL_SPEED)
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

void Siren::makeAttack(float deltaX, float deltaY)
{
	if (player->isFreeze()) return;

	if (deltaX < 128 && deltaY < 24)
	{
		_ani = _animations["STRIKE1"];
		_ani->reset();
		_isStanding = false;
		_isAttack = true;
		_isMirrored = player->position.x < position.x;

		WwdObject obj;
		obj.x = (int32_t)(position.x + (_isMirrored ? _saveCurrRect.left - _saveCurrRect.right : _saveCurrRect.right - _saveCurrRect.left));
		obj.y = (int32_t)position.y;
		obj.speedX = _isMirrored ? -100 : 100;
		for (int delay = 0; delay <= 1000; delay += 250)
			actionPlane->addPlaneObject(DBG_NEW SirenProjectile(obj, delay));

		_attackRest = 4500;
	}
}
