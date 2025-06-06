#include "StartElevator.h"
#include "../../GlobalObjects.h"
#include "../Player/Player.h"


StartElevator::StartElevator(const WwdObject& obj)
	: Elevator(obj)
{
	_operateElevator = false;
}
void StartElevator::Logic(uint32_t elapsedTime)
{
	if (GO::player->elevator != this)
		_operateElevator = tryCatchPlayer(); // if CC left the elevator it will not keep moving (until he comes up again)
	if (_operateElevator)
		Elevator::Logic(elapsedTime);
}
void StartElevator::Reset()
{
	Elevator::Reset();
	_operateElevator = false;
}
