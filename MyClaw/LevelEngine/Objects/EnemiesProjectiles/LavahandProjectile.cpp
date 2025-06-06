#include "LavahandProjectile.h"
#include "../OneTimeAnimation.h"
#include "../../GlobalObjects.h"


LavahandProjectile::LavahandProjectile(D2D1_POINT_2F initialPosition, bool mirrored)
	: EnemyProjectile(AssetsManager::loadAnimation(
		PathManager::getAnimationPath("LEVEL_LAVAHANDPROJECTILE"),
		PathManager::getImageSetPath("LEVEL_LAVAHANDPROJECTILE")),
		10, { mirrored ? 0.45f : -0.45f, -0.65f }, initialPosition)
{
}

LavahandProjectile::~LavahandProjectile()
{
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, "GAME_DYNAMITEEXPLO"));
	}
}

void LavahandProjectile::Logic(uint32_t elapsedTime)
{
	removeObject = speed.x == 0 && speed.y == 0;
	speed.y += GRAVITY * elapsedTime;
	position.x += speed.x * elapsedTime;
	position.y += speed.y * elapsedTime;
}

void LavahandProjectile::stopFalling(float collisionSize)
{
	speed = {};
	position.y -= collisionSize;
}
