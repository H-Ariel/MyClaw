#pragma once

#include "BasePlaneObject.h"


class TogglePeg : public BaseStaticPlaneObject
{
public:
	TogglePeg(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

	void exitEasyMode() override;

private:
	const int _startTimeDelay; // in milliseconds
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


class BreakPlank : public BaseStaticPlaneObject
{
public:
	BreakPlank(const WwdObject& obj, float topOffset);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	void Reset() override;

private:
	vector<shared_ptr<UIAnimation>> _planks;
	int activeIdx; // index of active plank
	float _topOffset; // the offset from the top of the image to the top of the plank
};
