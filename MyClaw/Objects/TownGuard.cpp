#include "TownGuard.h"

// TODO (?): add ability to block CC attacks
// TODO (?): add ability to jump back (escape from CC attacks)

TownGuard::TownGuard(const WwdObject& obj)
	: BaseEnemy(obj, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE2", "STRIKE1", "", "", "", 0.1f),
	_type((Type)(obj.logic[obj.logic.length() - 1] - '1'))
{
}

Rectangle2D TownGuard::GetRect()
{
	Rectangle2D rc;

	if (_type == Type::Guard1)
	{
		rc.left = position.x - 20;
		rc.right = position.x + 20;
		rc.top = position.y - 50;
		rc.bottom = position.y + 60;
	}
	else //if (_type == Type::Guard2)
	{
		rc.top = position.y - 50;
		rc.bottom = position.y + 70;

		if (_isMirrored)
		{
			rc.left = position.x - 10;
			rc.right = position.x + 30;
		}
		else
		{
			rc.left = position.x - 30;
			rc.right = position.x + 10;
		}
	}

	_saveCurrRect = rc;
	return rc;
}

pair<Rectangle2D, int> TownGuard::GetAttackRect()
{
	if (!_isAttack) return {};

	Rectangle2D rc;

	if (_isMirrored)
	{
		rc.left = -60;
		rc.right = 20;
	}
	else
	{
		rc.left = 30;
		rc.right = 110;
	}

	if (_ani == _animations["STRIKE2"]) // strike high
	{
		rc.top = 20;
		rc.bottom = 40;
	}
	else
	{
		rc.top = 80;
		rc.bottom = 100;
	}

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}
