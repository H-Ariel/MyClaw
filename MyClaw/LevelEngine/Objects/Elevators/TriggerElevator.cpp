#include "TriggerElevator.h"


TriggerElevator::TriggerElevator(const WwdObject& obj)
	: Elevator(obj)
{
	_operateElevator = false;
}

void TriggerElevator::Logic(uint32_t elapsedTime)
{
	if (_operateElevator)
		Elevator::Logic(elapsedTime);
	else
		_operateElevator = tryCatchPlayer(); // even if CC left the elevator it will keep moving
}
