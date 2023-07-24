#include "Chameleon.h"
#include "../Player.h"


// TODO: let it disapper (no animation for this. should use effect)


Chameleon::Chameleon(const WwdObject& obj)
	: BaseEnemy(obj, 1, 10, "FASTADVANCE", "HIT",
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

	if (_isMirrored)
	{
		rc.left = -90;
		rc.right = 10;
	}
	else
	{
		rc.left = 30;
		rc.right = 130;
	}

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}

void Chameleon::makeAttack()
{
	if (_isStanding || enemySeeClaw())
	{
		if (abs(player->position.x - position.x) < 128 && abs(player->position.y - position.y) < 32) // CC is close to enemy
		{
			_ani = _animations["STRIKE1"];
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;
		}
	}
}
