#pragma once

#include "Elevator.h"


class StartElevator : public Elevator
{
public:
	StartElevator(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;
};
