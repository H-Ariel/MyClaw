#pragma once

#include "BasePlaneObject.h"


class Laser : public BaseDamageObject
{
public:
	Laser(const WwdObject& obj);
	
	void Logic(uint32_t elapsedTime) override;
	bool isDamage() const override;

private:
	const int _swapTime; // in milliseconds
	int _timeCounter;
	bool _isActive;
};
