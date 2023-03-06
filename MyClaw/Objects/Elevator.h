#pragma once

#include "../BasePlaneObject.h"


class Elevator : public BaseDynamicPlaneObject
{
public:
	Elevator(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

private:
	void mainLogic(uint32_t elapsedTime); // logic for regular elevator
	bool tryCatchPlayer(); // returns `true` if successfully caught the player, else - `false`.

	const D2D1_POINT_2F _minPos, _maxPos;
	const D2D1_POINT_2F _initialPos, _initialSpeed;
	bool _arrivedToEdge; // a flag to indicate that elevator finish its movement (used in `OneWay...Elevator`)
	bool _operateElevator;
	const bool _isTriggerElevator;
	const bool _isStartElevator;
	const bool _isOneWayElevator;
};
