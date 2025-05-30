#pragma once

#include "../../../Objects/BasePlaneObject.h"


constexpr int NUM_OF_ITEMS_IN_SHIELD = 9;


class LordOmarShield : public BaseDynamicPlaneObject
{
public:
	LordOmarShield(D2D1_POINT_2F center, const string& shieldItemImageSet, bool rotateClockwise);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	bool isFalling() const override;
	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void bounceTop() override;
	void whenTouchDeath() override;

private:
	D2D1_POINT_2F getItemPosition(int i) const;

	const D2D1_POINT_2F _center;
	float _itemsAngles[NUM_OF_ITEMS_IN_SHIELD]; // save angle (in radians) for each item of the shield
	const bool _rotateClockwise;
};
