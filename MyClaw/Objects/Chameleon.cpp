#include "Chameleon.h"
#include "../Player.h"


// TODO: let it disapper (no animation for this. should use effect)


Chameleon::Chameleon(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 1, 10, "FASTADVANCE", "HIT",
		"HIT", "KILLFALL", "STRIKE1", "", "", "", "", 0.1f)
{
}

Rectangle2D Chameleon::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 25;
	_saveCurrRect.bottom = position.y + 35;
	return _saveCurrRect;
}

pair<Rectangle2D, int> Chameleon::GetAttackRect()
{
	Rectangle2D rc;

	rc.top = 10;
	rc.bottom = 30;

	if (_forward)
	{
		rc.left = 30;
		rc.right = 130;
	}
	else
	{
		rc.left = -90;
		rc.right = 10;
	}

	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}

void Chameleon::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		if (abs(_player->position.x - position.x) < 128 && abs(_player->position.y - position.y) < 32) // CC is close to enemy
		{
			_ani = _animations["STRIKE1"];
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;
		}
	}
}
