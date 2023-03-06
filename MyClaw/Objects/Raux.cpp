#include "Raux.h"


Raux::Raux(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 100, 10, "ADVANCE", "HITHIGH", "HITLOW",
		"STRIKE", "", "", { { "HOME", 1000 }, { "HOME1", 1000 } })
{
}

D2D1_RECT_F Raux::GetRect()
{
	_saveCurrRect = _ani->GetRect();
	return _saveCurrRect;
}
pair<D2D1_RECT_F, int8_t> Raux::GetAttackRect()
{
	GetRect();
	// TODO: write code
	return {};
}
