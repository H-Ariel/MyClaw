#include "RobberThief.h"


RobberThief::RobberThief(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL",
		"", "", "STRIKE1", "STRIKE2", PathManager::getImageSetPath("LEVEL_ARROW"), 0.1f)
{
}

pair<Rectangle2D, int8_t> RobberThief::GetAttackRect()
{
	return {}; // he shoot arrows, there is no attack we CC is close
}
