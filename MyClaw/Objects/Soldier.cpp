#include "Soldier.h"


Soldier::Soldier(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL", "STRIKE1", "", "STRIKE", "STRIKE2",
		PathManager::getImageSetPath("LEVEL_MUSKETBALL"), { { "STAND", 1000 }, { "STAND1", 1000 }, { "STAND2", 1000 } })
{
}

D2D1_RECT_F Soldier::GetRect()
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
		rc.left = 0;
		rc.top = 0;
		rc.right = rc.left + 50;
		rc.bottom = rc.top + 110;

		if (isDuck())
		{
			rc.top += 40;
		}
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
pair<D2D1_RECT_F, int8_t> Soldier::GetAttackRect()
{
	if (!_isAttack && _ani->getFrameNumber() != 5 && _ani->getFrameNumber() != 4) return {};

	D2D1_RECT_F rc = {};
	rc.top = 0;
	rc.bottom = 20;

	if (_forward)
	{
		rc.left = 65;
		rc.right = 95;
	}
	else
	{
		rc.left = -15;
		rc.right = 15;
	}

	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}
