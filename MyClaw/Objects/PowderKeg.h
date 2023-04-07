#pragma once

#include "../BasePlaneObject.h"


class PowderKeg : public BaseDynamicPlaneObject
{
public:
	PowderKeg(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	int8_t getDamage() const;

	bool raise(); // makes the powder keg state to `Raised` and returns `true` if it was `Stand`
	void thrown(bool forward); // makes the powder keg state to `Thrown`
	void fall(); // makes the powder keg state to `Thrown` but set no speed

	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void bounceTop() override;

private:
	bool shouldMakeExplos();

	enum class State : int8_t { Stand, Explos, Raised, Thrown };
	const string _imageSet;
	State _state;
};
