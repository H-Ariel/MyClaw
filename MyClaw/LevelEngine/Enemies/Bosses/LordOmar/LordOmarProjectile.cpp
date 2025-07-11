#include "LordOmarProjectile.h"


LordOmarProjectile::LordOmarProjectile(D2D1_POINT_2F pos, float speedX)
	: EnemyProjectile(AssetsManager::getAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_OMARPROJECTILE")), 20, { speedX , 0 }, pos)
{
}
Rectangle2D LordOmarProjectile::GetRect()
{
	return Rectangle2D(position.x - 16, position.y - 16, position.x + 16, position.y + 16);
}
