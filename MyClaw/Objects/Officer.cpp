#include "Officer.h"


Officer::Officer(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL", "STRIKE", "", "", "", "",
		{ { "STAND1", 1000 }, { "STAND2", 1000 }, { "STAND3", 1000 }, { "STAND4", 1000 }, { "STAND5", 1000 } })
{
}

D2D1_RECT_F Officer::GetRect()
{
	D2D1_RECT_F rc = {};

	if (isWalkAnimation() || _isStanding || isTakeDamage())
	{
		rc.left = 0;
		rc.top = 0;
		rc.right = rc.left + 50;
		rc.bottom = rc.top + 110;
	}
	else if (_isAttack)
	{
		if (_forward) rc.left = 20;
		else rc.left = -20;
		rc.top = 0;
		rc.right = rc.left + 50;
		rc.bottom = rc.top + 110;
	}
	else
	{
		rc = _ani->GetRect();
		rc.top -= position.y;
		rc.bottom -= position.y;
		rc.left -= position.x;
		rc.right -= position.x;
	}

	// set rectangle by center
	const float addX = position.x - (rc.right - rc.left) / 2, addY = position.y - (rc.bottom - rc.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	_saveCurrRect = rc;

	return rc;
}
pair<D2D1_RECT_F, int8_t> Officer::GetAttackRect()
{
	if (!_isAttack && _ani->getFrameNumber() < 3) return {};

	D2D1_RECT_F rc = {};
	rc.top = 20;
	rc.bottom = 40;

	if (_forward)
	{
		rc.left = 80;
		rc.right = 160;
	}
	else
	{
		rc.left = -110;
		rc.right = -30;
	}
	
	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}
