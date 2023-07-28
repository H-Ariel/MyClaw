#include "Soldier.h"


Soldier::Soldier(const WwdObject& obj)
	: BaseEnemy(obj, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL", "STRIKE1",
		"", "STRIKE", "STRIKE2", PathManager::getImageSetPath("LEVEL_MUSKETBALL"), ENEMY_PATROL_SPEED)
{
}

Rectangle2D Soldier::GetRect()
{
	if (isWalkAnimation() || _isStanding || isTakeDamage())
	{
		_saveCurrRect.left = 0;
		_saveCurrRect.top = 0;
		_saveCurrRect.right = 50;
		_saveCurrRect.bottom = 110;
	}
	else if (_isAttack)
	{
		if (isDuck())
		{
			_saveCurrRect.top = 16;
			_saveCurrRect.bottom = 80;
			_saveCurrRect.left = 10;
			_saveCurrRect.right = 60;
		}
		else
		{
			_saveCurrRect.left = 0;
			_saveCurrRect.top = 0;
			_saveCurrRect.right = 50;
			_saveCurrRect.bottom = 110;
		}
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
pair<Rectangle2D, int> Soldier::GetAttackRect()
{
	if (_ani->getFrameNumber() != 5 && _ani->getFrameNumber() != 4) return{};

	Rectangle2D rc;
	rc.top = 0;
	rc.bottom = 20;

	if (_isMirrored)
	{
		rc.left = -15;
		rc.right = 15;
	}
	else
	{
		rc.left = 65;
		rc.right = 95;
	}

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}
