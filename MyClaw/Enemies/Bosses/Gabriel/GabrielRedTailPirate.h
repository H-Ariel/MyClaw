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
	void stopMovingLeft(float collisionSize);
	void stopMovingRight(float collisionSize);

private:
	bool _isJumping;
};
