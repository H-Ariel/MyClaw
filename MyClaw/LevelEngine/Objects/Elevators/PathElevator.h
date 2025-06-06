#pragma once

#include "Elevator.h"


class PathElevator : public Elevator
{
public:
	PathElevator(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

private:
	vector<pair<int, int>> _paths; // { direction, delay (ms) }
	int _pathIdx, _timeCounter;
	const float _totalSpeed;
};
