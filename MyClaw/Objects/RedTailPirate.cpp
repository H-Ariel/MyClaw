#include "RedTailPirate.h"


RedTailPirate::RedTailPirate(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 12, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL",
		"STRIKE1", "STRIKE2", "", "", "", "IDLE")
{
}

D2D1_RECT_F RedTailPirate::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 50;
	_saveCurrRect.bottom = position.y + 60;
	return _saveCurrRect;
}

pair<D2D1_RECT_F, int8_t> RedTailPirate::GetAttackRect()
{
	if (!_isAttack) return {};

	D2D1_RECT_F rc = {};

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

	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}