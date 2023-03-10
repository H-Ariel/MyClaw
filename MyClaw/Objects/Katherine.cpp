#include "Katherine.h"


Katherine::Katherine(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 100, 10, "WALK", "HITHIGH", "HITLOW",
		"FALL", "STRIKE1", "", "", { { "IDLE", 1000 } })
{
}

D2D1_RECT_F Katherine::GetRect()
{
	_saveCurrRect = _ani->GetRect();
	return _saveCurrRect;
}
pair<D2D1_RECT_F, int8_t> Katherine::GetAttackRect()
{
	GetRect();
	// TODO: write code
	return {};
}
