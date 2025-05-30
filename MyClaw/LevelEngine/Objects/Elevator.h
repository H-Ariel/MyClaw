#pragma once

#include "BasePlaneObject.h"


class Elevator : public BaseDynamicPlaneObject
{
public:
	static Elevator* create(const WwdObject& obj, int levelNumber);

	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;
	void Reset() override;

protected:
	Elevator(const WwdObject& obj);
	void mainLogic(uint32_t elapsedTime);

	const D2D1_POINT_2F _initialPos, _initialSpeed;

protected: //private:
	bool tryCatchPlayer(); // returns `true` if successfully caught the player, else - `false`.
	
	const D2D1_POINT_2F _minPos, _maxPos;
	float _offsetY; // offset for player when he is on elevator
	bool _arrivedToEdge; // a flag to indicate that elevator finish its movement (used in `OneWay...Elevator`)
	bool _operateElevator;
	const bool _isOneWayElevator;
};
