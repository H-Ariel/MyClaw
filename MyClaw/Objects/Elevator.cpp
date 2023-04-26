#include "Elevator.h"
#include "../Player.h"
#include "../AssetsManager.h"


// The following classes indicate precisely the type of elevator:
// TriggerElevator: the elevator starts move when the player touches it (and never stops)
// StartElevator: the elevator starts move when the player touches it (and stops when he leaves it)
// PathElevator: the elevator moves along a path (defined in the logic field of the object)
class TriggerElevator : public Elevator
{
public:
	TriggerElevator(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
};
class StartElevator : public Elevator
{
public:
	StartElevator(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;
};
class PathElevator : public Elevator
{
public:
	PathElevator(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

private:
	vector<pair<int32_t, int32_t>> _paths; // { direction, delay (ms) }
	int _pathIdx, _timeCounter;
	const float _totalSpeed;
};


Elevator* Elevator::create(const WwdObject& obj, Player* player)
{
	if (obj.logic == "PathElevator")
		return DBG_NEW PathElevator(obj, player);
	if (contains(obj.logic, "Trigger"))
		return DBG_NEW TriggerElevator(obj, player);
	if (contains(obj.logic, "Start"))
		return DBG_NEW StartElevator(obj, player);

	return DBG_NEW Elevator(obj, player);
}


Elevator::Elevator(const WwdObject& obj, Player* player)
	: BaseDynamicPlaneObject(obj, player), _arrivedToEdge(false),
	_minPos({ (float)obj.minX, (float)obj.minY }), _maxPos({ (float)obj.maxX, (float)obj.maxY }),
	_initialPos({ (float)obj.x, (float)obj.y }), _initialSpeed({}),
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
}
void Elevator::Logic(uint32_t elapsedTime)
{
	if (_isOneWayElevator && _arrivedToEdge)
	{
		return;
	}

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

	mainLogic(elapsedTime);
}
void Elevator::mainLogic(uint32_t elapsedTime) // logic for every elevator
{
	const float deltaX = _speed.x * elapsedTime, deltaY = _speed.y * elapsedTime;

	position.x += deltaX;
	position.y += deltaY;

	if (_player->elevator == this)
	{
		const Rectangle2D thisRc = GetRect(), playerRc = _player->GetRect();

		// if no collision - disable the 'elevator mode' for player
		if (playerRc.right < thisRc.left || thisRc.right < playerRc.left)
		{
			_player->elevator = nullptr;
		}
		else
		{
			_player->position.x += deltaX;
			_player->position.y = position.y - 64; // TODO: fit offset to level elevator
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
	if (_isOneWayElevator)
		_operateElevator = false;
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


TriggerElevator::TriggerElevator(const WwdObject& obj, Player* player)
	: Elevator(obj, player)
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


StartElevator::StartElevator(const WwdObject& obj, Player* player)
	: Elevator(obj, player)
{
	_operateElevator = false;
}
void StartElevator::Logic(uint32_t elapsedTime)
{
	if (_player->elevator != this)
		_operateElevator = tryCatchPlayer(); // if CC left the elevator it will not keep moving (until he comes up again)
	if (_operateElevator)
		Elevator::Logic(elapsedTime);
}
void StartElevator::Reset()
{
	Elevator::Reset();
	_operateElevator = false;
}


PathElevator::PathElevator(const WwdObject& obj, Player* player)
	: Elevator(obj, player), _totalSpeed(obj.speed ? (obj.speed / 1000.f) : 0.125f)
{
	if (obj.moveRect.left != 0) _paths.push_back({ obj.moveRect.left, obj.moveRect.top });
	if (obj.moveRect.right != 0) _paths.push_back({ obj.moveRect.right, obj.moveRect.bottom });
	if (obj.hitRect.left != 0) _paths.push_back({ obj.hitRect.left, obj.hitRect.top });
	if (obj.hitRect.right != 0) _paths.push_back({ obj.hitRect.right, obj.hitRect.bottom });
	if (obj.attackRect.left != 0) _paths.push_back({ obj.attackRect.left, obj.attackRect.top });
	if (obj.attackRect.right != 0) _paths.push_back({ obj.attackRect.right, obj.attackRect.bottom });
	if (obj.clipRect.left != 0) _paths.push_back({ obj.clipRect.left, obj.clipRect.top });
	if (obj.clipRect.right != 0) _paths.push_back({ obj.clipRect.right, obj.clipRect.bottom });

	if (_paths.size() == 0) throw Exception("PathElevator - no paths");

	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 125, false);

	Reset();
}
void PathElevator::Logic(uint32_t elapsedTime)
{
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

	mainLogic(elapsedTime);
}
void PathElevator::Reset()
{
	position = _initialPos;
	_pathIdx = -1;
	_timeCounter = 0;
}
