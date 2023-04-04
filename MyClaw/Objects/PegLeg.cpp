#include "PegLeg.h"


// TODO: let him roll

PegLeg::PegLeg(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 12, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "", "", "STRIKE1", "STRIKE2", "GAME/IMAGES/BULLETS")
{
}

D2D1_RECT_F PegLeg::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 40;
	_saveCurrRect.bottom = position.y + 50;
	return _saveCurrRect;
}

pair<D2D1_RECT_F, int8_t> PegLeg::GetAttackRect()
{
	// the PegLeg use the pistol, so this stay empty
	return pair<D2D1_RECT_F, int8_t>();
}
