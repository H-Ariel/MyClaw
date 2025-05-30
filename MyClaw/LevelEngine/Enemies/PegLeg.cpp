#include "PegLeg.h"


// TODO: let him roll

PegLeg::PegLeg(const WwdObject& obj)
	: BaseEnemy(obj, 12, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "", "", "STRIKE1", "STRIKE2", "GAME_BULLETS", 0.15f)
{
}

Rectangle2D PegLeg::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 40;
	_saveCurrRect.bottom = position.y + 50;
	return _saveCurrRect;
}

pair<Rectangle2D, int> PegLeg::GetAttackRect()
{
	// the PegLeg use the pistol, so this stay empty
	return {};
}
