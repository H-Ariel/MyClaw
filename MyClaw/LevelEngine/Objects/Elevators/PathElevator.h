#pragma once

#include "Elevator.h"


class PathElevator : public Elevator
{
public:
	PathElevator(const WwdObject& obj);
	~PathElevator();
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

private:
	void changeDirection();

	vector<pair<int, int>> _paths; // { direction, delay (ms) }
	int _pathIdx;
	const float _totalSpeed;

	Timer* _timer;
};
