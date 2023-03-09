#include "RobberThief.h"


RobberThief::RobberThief(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL", "", "STRIKE1", "STRIKE2",
		PathManager::getImageSetPath("LEVEL_ARROW"), 
		{ { "IDLE1", 1000 }, { "IDLE2", 1000 }, { "IDLE3", 1000 }, { "IDLE4", 1000 } })
{
}

D2D1_RECT_F RobberThief::GetRect()
{
	_saveCurrRect = _ani->GetRect();
	return _saveCurrRect;
}
pair<D2D1_RECT_F, int8_t> RobberThief::GetAttackRect()
{
	return {};
}
