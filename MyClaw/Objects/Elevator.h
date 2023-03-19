#pragma once

#include "../BasePlaneObject.h"


class Elevator : public BaseDynamicPlaneObject
{
public:
	Elevator(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

protected:
	void mainMainLogic(uint32_t elapsedTime); // the real logic for every elevator. TODO: rename

	const D2D1_POINT_2F _initialPos, _initialSpeed;

private:
	void mainLogic(uint32_t elapsedTime); // logic for regular elevator
	bool tryCatchPlayer(); // returns `true` if successfully caught the player, else - `false`.
	
	const D2D1_POINT_2F _minPos, _maxPos;
	bool _arrivedToEdge; // a flag to indicate that elevator finish its movement (used in `OneWay...Elevator`)
	bool _operateElevator;
	const bool _isTriggerElevator, _isStartElevator, _isOneWayElevator;
};


class PathElevator : public Elevator
{
public:
	PathElevator(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

private:
	vector<pair<int32_t, int32_t>> _paths; // { direction, delay (ms) }
	size_t _pathIdx;
	int32_t _timeCounter;
	float _totalSpeed;
	const float _initialSpeed;
};
