#include "Officer.h"


Officer::Officer(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH",
		"HITLOW", "FALL", "STRIKE", "", "", "", "", 0.1f)
{
}

Rectangle2D Officer::GetRect()
{
	if (isWalkAnimation() || _isStanding || isTakeDamage())
	{
		_saveCurrRect.left = 0;
		_saveCurrRect.top = 0;
		_saveCurrRect.right = _saveCurrRect.left + 50;
		_saveCurrRect.bottom = _saveCurrRect.top + 110;
	}
	else if (_isAttack)
	{
		if (_isMirrored) _saveCurrRect.left = -20;
		else _saveCurrRect.left = 20;
		_saveCurrRect.top = 0;
		_saveCurrRect.right = _saveCurrRect.left + 50;
		_saveCurrRect.bottom = _saveCurrRect.top + 110;
	}
	else
	{
		_saveCurrRect = _ani->GetRect();
		_saveCurrRect.top -= position.y;
		_saveCurrRect.bottom -= position.y;
		_saveCurrRect.left -= position.x;
		_saveCurrRect.right -= position.x;
	}

	_saveCurrRect = setRectByCenter(_saveCurrRect);
	return _saveCurrRect;
}
pair<Rectangle2D, int> Officer::GetAttackRect()
{
	if (_ani->getFrameNumber() != 3) return {};

	Rectangle2D rc;
	rc.top = 20;
	rc.bottom = 40;

	if (_isMirrored)
	{
		rc.left = -110;
		rc.right = -30;
	}
	else
	{
		rc.left = 80;
		rc.right = 160;
	}
	
	return { setRectByCenter(rc, _saveCurrRect), _damage };
}
