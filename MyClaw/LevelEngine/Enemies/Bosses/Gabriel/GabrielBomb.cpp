#include "GabrielBomb.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/OneTimeAnimation.h"


GabrielBomb::GabrielBomb(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getAnimationPath("LEVEL_GABRIELBOMB_FALL"),
		PathManager::getImageSetPath("LEVEL_GABRIELBOMB"))
{
}
GabrielBomb::~GabrielBomb()
{
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, "LEVEL_GABRIELBOMB_EXPLODE", "LEVEL_GABRIELBOMB"));
	}
}
void GabrielBomb::Logic(uint32_t elapsedTime)
{
	if (!removeObject)
		speed.y += GRAVITY * elapsedTime;
	Projectile::Logic(elapsedTime);
}
void GabrielBomb::stopFalling(float collisionSize)
{
	position.y -= collisionSize;
	position.y -= 32;
	removeObject = true;
}
