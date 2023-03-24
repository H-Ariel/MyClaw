#pragma once

#include "../BasePlaneObject.h"


class FloorSpike : public BaseStaticPlaneObject
{
public:
	FloorSpike(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	int8_t getDamage() const;

protected:
	shared_ptr<Animation> _aniAppear, _aniDisappear;
	enum class States { Appear, WaitAppear, Disappear, WaitDisappear };
	States _state;
	int32_t _totalTime; // in milliseconds
	int32_t _timeOn, _timeOff; // in milliseconds
	int32_t _startTimeDelay; // in milliseconds
};


class SawBlade : public FloorSpike
{
public:
	SawBlade(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _aniWait;
};
