#include "CutThroat.h"


CutThroat::CutThroat(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL", "STRIKE1", "STRIKE3", "STRIKE2",
		PathManager::getImageSetPath("LEVEL_KNIFE"), { { "STAND1", 1000 }, { "STAND2", 1000 }, { "STAND3", 1000 }, { "STAND4", 1000 } })
{
}

D2D1_RECT_F CutThroat::GetRect()
{
	_saveCurrRect = _ani->GetRect();
	return _saveCurrRect;
}
pair<D2D1_RECT_F, int8_t> CutThroat::GetAttackRect()
{
	D2D1_RECT_F rc = {};

	if (_isAttack)
	{
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

		rc.top = 30;
		rc.bottom = 50;
	}


	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}