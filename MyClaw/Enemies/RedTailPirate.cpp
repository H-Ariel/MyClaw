#include "RedTailPirate.h"


RedTailPirate::RedTailPirate(const WwdObject& obj, bool isFromGabriel)
	: BaseEnemy(obj, 12, 10, "FASTADVANCE", "HITHIGH",
		"HITLOW", "KILLFALL", "STRIKE1", "STRIKE2", "", "", "", 0.2f, isFromGabriel)
{
}

Rectangle2D RedTailPirate::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 50;
	_saveCurrRect.bottom = position.y + 60;
	return _saveCurrRect;
}

pair<Rectangle2D, int> RedTailPirate::GetAttackRect()
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

	if (_ani == _animations["STRIKE1"]) // strike high
	{
		rc.top = 20;
		rc.bottom = 40;
	}
	else
	{
		rc.top = 80;
		rc.bottom = 100;
	}

	return { setRectByCenter(rc,_saveCurrRect), _damage };
}
