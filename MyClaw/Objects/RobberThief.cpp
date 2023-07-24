#include "RobberThief.h"


RobberThief::RobberThief(const WwdObject& obj)
	: BaseEnemy(obj, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "FALL",
		"", "", "STRIKE1", "STRIKE2", PathManager::getImageSetPath("LEVEL_ARROW"), 0.1f)
{
}

pair<Rectangle2D, int> RobberThief::GetAttackRect()
{
	return {}; // he shoot arrows, there is no attack we CC is close
}
