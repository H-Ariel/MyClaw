#include "RatBomb.h"
#include "../OneTimeAnimation.h"
#include "../../GlobalObjects.h"


RatBomb::RatBomb(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getAnimationPath("LEVEL_RATBOMB_FALLEASTWEST"))
{
	_isMirrored = !_isMirrored; // the rat-bomb is already mirrored
}

RatBomb::~RatBomb()
{
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, "LEVEL_RATBOMB_EXPLODE"));
	}
}
