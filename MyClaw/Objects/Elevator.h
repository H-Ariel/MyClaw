#pragma once

#include "../BasePlaneObject.h"


class Elevator : public BaseDynamicPlaneObject
{
public:
	static Elevator* create(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

protected:
	Elevator(const WwdObject& obj, Player* player); // protected ctor to prevent direct instantiation
	void mainLogic(uint32_t elapsedTime);

	const D2D1_POINT_2F _initialPos, _initialSpeed;

protected: //private:
	bool tryCatchPlayer(); // returns `true` if successfully caught the player, else - `false`.
	
	const D2D1_POINT_2F _minPos, _maxPos;
	bool _arrivedToEdge; // a flag to indicate that elevator finish its movement (used in `OneWay...Elevator`)
	bool _operateElevator;
	const bool _isOneWayElevator;
};



