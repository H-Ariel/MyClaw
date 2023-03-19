#include "Elevator.h"
#include "../Player.h"
#include "../AssetsManager.h"


Elevator::Elevator(const WwdObject& obj, Player* player)
	: BaseDynamicPlaneObject(obj, player), _arrivedToEdge(false),
	_minPos({ (float)obj.minX, (float)obj.minY }), _maxPos({ (float)obj.maxX, (float)obj.maxY }),
	_initialPos({ (float)obj.x, (float)obj.y }), _initialSpeed({}),
	_isTriggerElevator(contains(obj.logic, "Trigger")),
	_isStartElevator(contains(obj.logic, "Start")),
	_isOneWayElevator(contains(obj.logic, "OneWay")),
	_operateElevator(true)
{
	_ani = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");

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
	if ((_speed.x < 0 && position.x < _minPos.x) || (_speed.x > 0 && position.x > _maxPos.x))
	{
		_speed.x = -_speed.x;
		_speed.y = -_speed.y;
		_arrivedToEdge = true;
	}

	if ((_speed.y < 0 && position.y < _minPos.y) || (_speed.y > 0 && position.y > _maxPos.y))
	{
		_speed.y = -_speed.y;
		_arrivedToEdge = true;
	}

	mainMainLogic(elapsedTime);
}
void Elevator::mainMainLogic(uint32_t elapsedTime)
{
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


PathElevator::PathElevator(const WwdObject& obj, Player* player)
	: Elevator(obj, player), _initialSpeed(0)
{
	if (obj.moveRect.left != 0) _paths.push_back({ obj.moveRect.left, obj.moveRect.top });
	if (obj.moveRect.right != 0) _paths.push_back({ obj.moveRect.right, obj.moveRect.bottom });
	if (obj.hitRect.left != 0) _paths.push_back({ obj.hitRect.left, obj.hitRect.top });
	if (obj.hitRect.right != 0) _paths.push_back({ obj.hitRect.right, obj.hitRect.bottom });
	if (obj.attackRect.left != 0) _paths.push_back({ obj.attackRect.left, obj.attackRect.top });
	if (obj.attackRect.right != 0) _paths.push_back({ obj.attackRect.right, obj.attackRect.bottom });
	if (obj.clipRect.left != 0) _paths.push_back({ obj.clipRect.left, obj.clipRect.top });
	if (obj.clipRect.right != 0) _paths.push_back({ obj.clipRect.right, obj.clipRect.bottom });

	if (_paths.size() == 0)
	{
		throw Exception(__FUNCTION__ " - no paths");
	}

	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 125, false);

	_totalSpeed = obj.speed / 1000.f;
	if (_totalSpeed == 0) _totalSpeed = 0.125f;

	myMemCpy(_initialSpeed, _totalSpeed);

	Reset();
}
void PathElevator::Logic(uint32_t elapsedTime)
{
	// TODO: something wrong when `WindowManager::PixelSize` changed

	_timeCounter -= elapsedTime;

	if (_timeCounter <= 0)
	{
		_pathIdx = (_pathIdx + 1) % _paths.size();
		_timeCounter = int32_t(_paths[_pathIdx].second / _totalSpeed);
		switch (_paths[_pathIdx].first)
		{
		case WwdObject::Direction_BottomLeft:	_speed = { -_totalSpeed, _totalSpeed }; break;
		case WwdObject::Direction_Down:			_speed = { 0, _totalSpeed }; break;
		case WwdObject::Direction_BottomRight:	_speed = { _totalSpeed, _totalSpeed }; break;
		case WwdObject::Direction_Left:			_speed = { -_totalSpeed, 0 }; break;
		case WwdObject::Direction_Right:		_speed = { _totalSpeed, 0 }; break;
		case WwdObject::Direction_TopLeft:		_speed = { -_totalSpeed, -_totalSpeed }; break;
		case WwdObject::Direction_Up:			_speed = { 0, -_totalSpeed }; break;
		case WwdObject::Direction_TopRight:		_speed = { _totalSpeed, -_totalSpeed }; break;
		default: _speed = {}; break;
		}
	}

	mainMainLogic(elapsedTime);
}
void PathElevator::Reset()
{
	position = _initialPos;
	_totalSpeed = _initialSpeed;
	_pathIdx = -1;
	_timeCounter = 0;
}
