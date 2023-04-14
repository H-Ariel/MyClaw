#pragma once

#include "BasePlaneObject.h"
#include "WwdFile.h"


class PhysicsManager
{
public:
	static const float myGRAVITY;

	PhysicsManager(const WwdPlane& plane, const shared_ptr<WapWorld>& wwd, Player*& player);

	void init();

	void checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath);

	void Draw();

private:
	vector<Rectangle2D> _solidRects, _groundRects, _deathRects, _climbRects;
	const WwdPlane& _plane;
	const shared_ptr<WapWorld>& _wwd;
	Player*& _player;
};
