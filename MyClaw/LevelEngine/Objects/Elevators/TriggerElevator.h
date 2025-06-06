#pragma once

#include "Elevator.h"


class TriggerElevator : public Elevator
{
public:
	TriggerElevator(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
};
