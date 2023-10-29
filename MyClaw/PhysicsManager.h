#pragma once

#include "BasePlaneObject.h"
#include "Assets-Managers/WwdFile.h"


class PhysicsManager
{
public:
	PhysicsManager(WapWorld* wwd, const LevelPlane* plane);

	void checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath) const;
	pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;

	void Draw();

private:
	vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }
};
