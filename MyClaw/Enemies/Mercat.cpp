#include "Mercat.h"
#include "../Player.h"
#include "../ActionPlane.h"
#include "../Objects/EnemyProjectile.h"

// TODO: jump back when CC try to attack

Mercat::Mercat(const WwdObject& obj)
	: BaseEnemy(obj, 15, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "", "", "", "", "", ENEMY_PATROL_SPEED)
{
}

Rectangle2D Mercat::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 50;
	_saveCurrRect.bottom = position.y + 50;
	return _saveCurrRect;
}

pair<Rectangle2D, int> Mercat::GetAttackRect()
{
	if (!_isAttack) return {};

	Rectangle2D rc;

	if (!_isMirrored)
	{
		rc.left = 30;
		rc.right = 110;
	}
	else
	{
		rc.left = -60;
		rc.right = 20;
	}

	rc.top = 40;
	rc.bottom = 60;

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}

void Mercat::makeAttack(float deltaX, float deltaY)
{
	if (deltaY < 24)
	{
		bool strike = false;

		if (deltaX < 128) // CC is close to enemy
		{
			strike = true;
		}
		else if (deltaX < 256) // CC is far from enemy
		{
			strike = true;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (_isMirrored ? _saveCurrRect.left - _saveCurrRect.right : _saveCurrRect.right - _saveCurrRect.left));
			obj.y = (int32_t)position.y;
			obj.speedX = _isMirrored ? -DEFAULT_PROJECTILE_SPEED : DEFAULT_PROJECTILE_SPEED;
			obj.damage = 10;
			actionPlane->addPlaneObject(DBG_NEW MercatTrident(obj));
		}

		if (strike)
		{
			_ani = _animations["STRIKE1"];
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;

			_attackRest = 2500;
		}
	}
}
