#pragma once

#include "BasePlaneObject.h"
#include "RezParser/WwdFile.h"

class LevelPlane;


class PhysicsManager
{
public:
	PhysicsManager(WapWwd* wwd, const LevelPlane* plane);

	void moveX(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const;
	void moveY(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const;
	void move(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const;
	void checkCollides(BaseDynamicPlaneObject* obj) const;
	pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;

private:
	vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }
};
