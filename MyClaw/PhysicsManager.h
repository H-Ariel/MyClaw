#pragma once

#include "BasePlaneObject.h"
#include "WwdFile.h"


class PhysicsManager
{
public:
	static const float myGRAVITY;

	PhysicsManager(const WwdPlane& plane, const shared_ptr<WapWorld>& wwd, Player*& player);

	void checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath);


private:
	const WwdPlane& _plane;
	const shared_ptr<WapWorld>& _wwd;
	Player*& _player;
};
