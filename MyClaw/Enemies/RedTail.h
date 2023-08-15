#pragma once

#include "../BaseEnemy.h"


class RedTailSpikes : public BaseDamageObject
{
public:
	RedTailSpikes(const WwdObject& obj);
	bool isDamage() const override;
};


class RedTailWind : public BasePlaneObject
{
public:
	RedTailWind(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void activate();

private:
	int _windTimeCounter; // time left to wind
};
