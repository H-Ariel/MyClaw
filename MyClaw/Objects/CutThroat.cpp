#include "CutThroat.h"


CutThroat::CutThroat(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL",
		"STRIKE1", "", "STRIKE3", "STRIKE2", PathManager::getImageSetPath("LEVEL_KNIFE"))
{
}

pair<D2D1_RECT_F, int8_t> CutThroat::GetAttackRect()
{
	if (_ani->getFrameNumber() != 2) return {};

	D2D1_RECT_F rc = {};

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

	// set rectangle by center
	const float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2, addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	return { rc, _damage };
}
