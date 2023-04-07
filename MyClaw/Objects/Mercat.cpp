#include "Mercat.h"
#include "../Player.h"
#include "../ActionPlane.h"

// TODO: jump back when CC try to attack

Mercat::Mercat(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 15, 10, "FASTADVANCE", "HITHIGH",
		"HITLOW", "KILLFALL", "", "", "", "", "")
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

pair<Rectangle2D, int8_t> Mercat::GetAttackRect()
{
	if (!_isAttack) return {};

	Rectangle2D rc;

	if (_forward)
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

	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}

void Mercat::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);

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
				obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
				obj.y = (int32_t)position.y;
				obj.z = ZCoord;
				obj.speedX = _forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
				obj.damage = 10;
				ActionPlane::addPlaneObject(DBG_NEW MercatTrident(obj));
			}

			if (strike)
			{
				_ani = _animations["STRIKE1"];
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;

				_attackRest = 250;
			}
		}
	}
}
