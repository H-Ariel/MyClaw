#include "PhysicsManager.h"


PhysicsManager::PhysicsManager(const DynamicArray<DynamicArray<int32_t>>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions)
	: quadTree(tilesMap, tileDescriptions)
{
}

// TODO: use these functions. the problem: CC can't jump. need check for all objects
void PhysicsManager::moveX(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const
{
	obj->position.x += obj->speed.x * elapsedTime;
	checkCollides(obj);
}
void PhysicsManager::moveY(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const
{
	obj->position.y += obj->speed.y * elapsedTime;
	if (obj->speed.y > MAX_Y_SPEED)
		obj->speed.y = MAX_Y_SPEED;
	checkCollides(obj);
}
void PhysicsManager::move(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const
{
	obj->position.x += obj->speed.x * elapsedTime;
	obj->position.y += obj->speed.y * elapsedTime;
	checkCollides(obj);
}

void PhysicsManager::checkCollides(BaseDynamicPlaneObject* obj) const
{
	quadTree.checkCollides(obj);
}

pair<float, float> PhysicsManager::getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const
{
	return quadTree.getEnemyRange(enemyPos, minX, maxX);
}
