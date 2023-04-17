#pragma once

#include "BasePlaneObject.h"
#include "WwdFile.h"


class PhysicsManager
{
public:
	static const float myGRAVITY;

	PhysicsManager(const WwdPlane& plane, const shared_ptr<WapWorld>& wwd, Player*& player);

	void init(int levelNumber);

	void checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath);

	void Draw();

private:
	vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }
	const WwdPlane& _plane;
	const shared_ptr<WapWorld>& _wwd;
	Player*& _player;
};
