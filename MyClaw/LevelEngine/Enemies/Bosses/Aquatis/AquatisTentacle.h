#pragma once

#include "../../../Objects/BasePlaneObject.h"


class AquatisTentacle : public BaseDamageObject
{
public:
	~AquatisTentacle();
	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;
	bool isDamage() const override;

private:
	AquatisTentacle(const WwdObject& obj); // create tentacles through Aquatis

	bool checkForHurts();
	bool checkForHurt(pair<Rectangle2D, int> hurtData, const Rectangle2D& thisRc);

	void respawn();

	shared_ptr<UIAnimation> _idle, _hit, _killfall, _respawn,
		_slap, // tentacle slap/whip (when player is too far)
		_squeeze; // tentacle squeeze (when player is too close)
	Rectangle2D _lastAttackRect;
	Timer _squeezeTimer; // timer for delay before squeeze CC again
	Timer _deadTimer; // timer for delay before it raised again after CC hit it

	friend class Aquatis;
};
