#pragma once

#include "BasePlaneObject.h"
#include "WwdFile.h"


class PhysicsManager
{
public:
	static const float myGRAVITY;

	PhysicsManager();

	void init(const WwdPlane* plane, WapWorld* wwd, Player* player, int levelNumber);

	void checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath);

	pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;

	void Draw();

private:
	vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }
	Player* _player;
};
