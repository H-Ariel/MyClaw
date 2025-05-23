#pragma once

#include "../../../Objects/BasePlaneObject.h"


// A pirate that Gabriel sends
// they run on dock, jump, and then behave like a regular character
class GabrielRedTailPirate : public BaseDynamicPlaneObject
{
public:
	GabrielRedTailPirate();
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;

private:
	bool _isJumping;
};
