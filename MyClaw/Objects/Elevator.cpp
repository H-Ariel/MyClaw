#include "Elevator.h"
#include "../Player.h"
#include "../AssetsManager.h"
#include "../PathManager.h"


Elevator::Elevator(const WwdObject& obj, Player* player)
	: BaseDynamicPlaneObject(obj, player), _arrivedToEdge(false),
	_minPos({ (float)obj.minX, (float)obj.minY }), _maxPos({ (float)obj.maxX, (float)obj.maxY }),
	_initialPos({ (float)obj.x, (float)obj.y }), _initialSpeed({}),
	_isTriggerElevator(contains(obj.logic, "Trigger")),
	_isStartElevator(contains(obj.logic, "Start")),
	_isOneWayElevator(contains(obj.logic, "OneWay")),
	_operateElevator(true)
{
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet));

	if (obj.minX > 0 && obj.maxX > 0)
	{
		if (obj.speedX > 0)
		{
			_speed.x = (float)obj.speedX;
		}
		else
		{
			_speed.x = 125;
		}
	}
	if (obj.minY > 0 && obj.maxY > 0)
	{
		if (obj.speedY > 0)
		{
			_speed.y = (float)obj.speedY;
		}
		else if (obj.speedX > 0)
		{
			_speed.y = (float)obj.speedX;
		}
		else
		{
			_speed.y = 125;
		}
	}

	switch (obj.direction)
	{
	case WwdObject::Direction_BottomLeft:	_speed = { -abs(_speed.x), abs(_speed.y) }; break;
	case WwdObject::Direction_Down:			_speed = { 0, abs(_speed.y) }; break;
	case WwdObject::Direction_BottomRight:	_speed = { abs(_speed.x), abs(_speed.y) }; break;
	case WwdObject::Direction_Left:			_speed = { -abs(_speed.x), 0 }; break;
	case WwdObject::Direction_NoMove:		_speed = { 0, 0 }; break;
	case WwdObject::Direction_Right:		_speed = { abs(_speed.x), 0 }; break;
	case WwdObject::Direction_TopLeft:		_speed = { -abs(_speed.x), -abs(_speed.y) }; break;
	case WwdObject::Direction_Up:			_speed = { 0, -abs(_speed.y) }; break;
	case WwdObject::Direction_TopRight:		_speed = { abs(_speed.x), -abs(_speed.y) }; break;
	default: break;
	}

	_speed.x /= 1000;
	_speed.y /= 1000;

	myMemCpy(_initialSpeed, _speed);

	if (_isTriggerElevator || _isStartElevator)
	{
		_operateElevator = false;
	}
}
void Elevator::Logic(uint32_t elapsedTime)
{
	if (_isOneWayElevator && _arrivedToEdge)
	{
		return;
	}

	if (_isTriggerElevator)
	{
		if (_operateElevator)
		{
			mainLogic(elapsedTime);
		}
		else
		{
			// even if Claw has left the elevator it will keep moving
			_operateElevator = tryCatchPlayer();
		}
	}
	else if (_isStartElevator)
	{
		if (_player->elevator != this)
		{
			// if Claw has left the elevator it will not keep moving (until he comes up again)
			_operateElevator = tryCatchPlayer();
		}

		if (_operateElevator)
		{
			mainLogic(elapsedTime);
		}
	}
	else
	{
		mainLogic(elapsedTime);
	}
}
void Elevator::mainLogic(uint32_t elapsedTime)
{
	bool directionChanged = false;

	// TODO: improve this part
	if (_speed.x < 0 && position.x < _minPos.x)
	{
		_speed.x = -_speed.x;
		directionChanged = true;
	}
	else if (_speed.x > 0 && position.x > _maxPos.x)
	{
		_speed.x = -_speed.x;
		directionChanged = true;
	}

	if (directionChanged)
	{
		_speed.y = -_speed.y;
	}
	else if (_speed.y < 0 && position.y < _minPos.y)
	{
		_speed.y = -_speed.y;
		directionChanged = true;
	}
	else if (_speed.y > 0 && position.y > _maxPos.y)
	{
		_speed.y = -_speed.y;
		directionChanged = true;
	}

	if (directionChanged)
	{
		_arrivedToEdge = true;
	}
	/////////////////////////

	const float deltaX = _speed.x * elapsedTime;
	const float deltaY = _speed.y * elapsedTime;

	position.x += deltaX;
	position.y += deltaY;

	if (_player->elevator == this)
	{
		const D2D1_RECT_F thisRc = GetRect(), playerRc = _player->GetRect();

		// if no collision - disable the 'elevator mode' for player
		if (playerRc.right < thisRc.left || thisRc.right < playerRc.left)
		{
			_player->elevator = nullptr;
		}
		else
		{
			_player->position.x += deltaX;
			_player->position.y = position.y - 64;
		}
	}
	else
	{
		tryCatchPlayer();
	}
}
void Elevator::Reset()
{
	position = _initialPos;
	_speed = _initialSpeed;
	_arrivedToEdge = false;
	if (_isStartElevator)
	{
		_operateElevator = false;
	}
}
bool Elevator::tryCatchPlayer()
{
	if (BaseDynamicPlaneObject::tryCatchPlayer())
	{
		_player->elevator = this;
		return true;
	}
	return false;
}
