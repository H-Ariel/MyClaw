#include "BearSailor.h"

/*
STRIKE1 - hand up
STRIKE2 - send hand to catch CC
STRIKE3 - catch CC
STRIKE4 - I couldn't figure it out
*/


BearSailor::BearSailor(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 12, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL",
		"STRIKE1", "", "", "", "", { { "IDLE", 1000 } })
{
}

D2D1_RECT_F BearSailor::GetRect()
{
	_saveCurrRect.left = position.x - 25;
	_saveCurrRect.right = position.x + 25;
	_saveCurrRect.top = position.y - 55;
	_saveCurrRect.bottom = position.y + 65;
	return _saveCurrRect;
}

pair<D2D1_RECT_F, int8_t> BearSailor::GetAttackRect()
{
	return {};
}
