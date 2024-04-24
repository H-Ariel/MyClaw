#pragma once

#include "../BasePlaneObject.h"


class TogglePeg : public BaseStaticPlaneObject
{
public:
	TogglePeg(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	int _startTimeDelay; // in milliseconds
};


class StartSteppingStone : public BaseStaticPlaneObject
{
public:
	StartSteppingStone(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
};


class CrumblingPeg : public BaseStaticPlaneObject
{
public:
	CrumblingPeg(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	void Reset() override;
};


class BreakPlank : public CrumblingPeg
{
public:
	BreakPlank(const WwdObject& obj, float topOffset); // topOffset is the offset from the top of the image to the top of the plank
};
