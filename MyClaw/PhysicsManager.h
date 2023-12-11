#pragma once

#include "BasePlaneObject.h"
#include "Assets-Managers/WwdFile.h"


class PhysicsManager
{
public:
	PhysicsManager(WapWorld* wwd, const LevelPlane* plane);

	void moveX(BaseDynamicPlaneObject* obj, float d) const;
	void moveY(BaseDynamicPlaneObject* obj, float d) const;
	void checkCollides(BaseDynamicPlaneObject* obj) const;
	pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;

private:
	vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }

	// TODO: save `const LevelPlane* plane`, not these fields

	vector<vector<int32_t>> tiles; // tiles[y][x] = id
	vector<WwdTileDescription> tilesDescription; // [id]=description
	const int tilesOnAxisX, tilesOnAxisY;
};
