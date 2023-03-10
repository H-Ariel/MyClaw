#pragma once

#include "../BasePlaneObject.h"


class TogglePeg : public BaseStaticPlaneObject
{
public:
	TogglePeg(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _aniAppear, _aniDisappear;
	enum class States { Appear, WaitAppear, Disappear, WaitDisappear };
	States _state;
	int32_t _totalTime; // in milliseconds
	int32_t _timeOn, _timeOff; // in milliseconds
	int32_t _startTimeDelay; // in milliseconds
};


class CrumblingPeg : public BaseStaticPlaneObject
{
public:
	CrumblingPeg(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	void Reset() override;

private:
	enum class States : int8_t { Appear, Wait, Crumbling };
	const int32_t _delayTime; // in milliseconds
	int32_t _timeCounter;
	States _state;
	bool _used, _draw;
};


class BreakPlank : public CrumblingPeg
{
public:
	BreakPlank(const WwdObject& obj, Player* player, WwdRect tileRc);
};
