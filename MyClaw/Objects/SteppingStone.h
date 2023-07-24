#pragma once

#include "../BasePlaneObject.h"


class SteppingStone : public BaseStaticPlaneObject
{
public:
	SteppingStone(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _aniAppear, _aniDisappear;
	enum class States { Appear, WaitAppear, Disappear, WaitDisappear };
	States _state;
	int32_t _totalTime; // in milliseconds
	int32_t _timeOn, _timeOff; // in milliseconds
	int32_t _startTimeDelay; // in milliseconds
};


class StartSteppingStone : public BaseStaticPlaneObject
{
public:
	StartSteppingStone(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _aniAppear, _aniDisappear;
	enum class States { Appear, WaitAppear, Disappear, WaitDisappear };
	States _state;
	int32_t _totalTime; // in milliseconds
	int32_t _timeOn, _timeOff; // in milliseconds
};