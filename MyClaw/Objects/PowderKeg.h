#pragma once

#include "../BasePlaneObject.h"


class PowderKeg : public BaseDynamicPlaneObject
{
public:
	PowderKeg(const WwdObject& obj);
	
	void Logic(uint32_t elapsedTime) override;

	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void bounceTop() override;

	int getDamage() const;

	bool raise(); // makes the powder keg state to `Raised` and returns `true` if it was `Stand`
	void thrown(bool forward); // makes the powder keg state to `Thrown`
	void fall(); // makes the powder keg state to `Thrown` but set no speed
	bool isStartExplode() const;

private:
	bool shouldMakeExplos();

	enum class State : int8_t { Stand, Explos, Raised, Thrown };
	const string _imageSet;
	State _state;
};
