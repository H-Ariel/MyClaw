#include "CrabBomb.h"
#include "../OneTimeAnimation.h"
#include "../../GlobalObjects.h"


CrabBomb::CrabBomb(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getAnimationPath("LEVEL_CRABBOMB_FALL"),
		PathManager::getImageSetPath("LEVEL_CRABBOMB"))
{
}
CrabBomb::~CrabBomb()
{
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, "LEVEL_CRABBOMB_EXPLODE", "LEVEL_CRABBOMB"));
	}
}
