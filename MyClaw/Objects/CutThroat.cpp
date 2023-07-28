#include "CutThroat.h"


CutThroat::CutThroat(const WwdObject& obj)
	: BaseEnemy(obj, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL",
		"STRIKE1", "", "STRIKE3", "STRIKE2", PathManager::getImageSetPath("LEVEL_KNIFE"), ENEMY_PATROL_SPEED)
{
}

pair<Rectangle2D, int> CutThroat::GetAttackRect()
{
	if (_ani->getFrameNumber() != 2) return {};

	Rectangle2D rc;
	rc.top = 30;
	rc.bottom = 50;

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
