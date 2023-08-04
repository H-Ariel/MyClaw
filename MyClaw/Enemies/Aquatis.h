#pragma once

#include "../BasePlaneObject.h"


class Tentacle : public BaseDamageObject
{
public:
	Tentacle(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

	Rectangle2D GetRect() override;

	bool isDamage() const override;

private:
	bool checkForHurts();

	shared_ptr<Animation> _idle, _hit, _killfall, _respawn,
		_slap, // tentacle slap/whip (when player is too far)
		_squeeze; // tentacle squeeze (when player is too close)

	Rectangle2D _lastAttackRect;
};
