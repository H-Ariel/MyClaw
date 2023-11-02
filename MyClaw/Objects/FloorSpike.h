#pragma once

#include "../BasePlaneObject.h"


class FloorSpike : public BaseDamageObject
{
public:
	FloorSpike(const WwdObject& obj, bool doFloorSpikeCtor = true);

	void Logic(uint32_t elapsedTime) override;
	bool isDamage() const override;

protected:
	int32_t _startTimeDelay; // in milliseconds
};


class SawBlade : public FloorSpike
{
public:
	SawBlade(const WwdObject& obj);

	bool isDamage() const override;
};

class LavaGeyser : public FloorSpike
{
public:
	LavaGeyser(const WwdObject& obj);
};
