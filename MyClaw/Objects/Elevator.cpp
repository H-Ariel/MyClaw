#include "Elevator.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"


// The following classes indicate precisely the type of elevator:
// TriggerElevator: the elevator starts move when the player touches it (and never stops)
// StartElevator: the elevator starts move when the player touches it (and stops when he leaves it)
// PathElevator: the elevator moves along a path (defined in the logic field of the object)
class TriggerElevator : public Elevator
{
public:
	TriggerElevator(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
};
class StartElevator : public Elevator
{
public:
	StartElevator(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;
};
class PathElevator : public Elevator
{
public:
	PathElevator(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

private:
	vector<pair<int32_t, int32_t>> _paths; // { direction, delay (ms) }
	int _pathIdx, _timeCounter;
	const float _totalSpeed;
};


Elevator* Elevator::create(const WwdObject& obj, int levelNumber)
{
	Elevator* elevator = nullptr;

	if (obj.logic == "PathElevator")
		elevator = DBG_NEW PathElevator(obj);
	else if (contains(obj.logic, "Trigger"))
		elevator = DBG_NEW TriggerElevator(obj);
	else if (contains(obj.logic, "Start"))
		elevator = DBG_NEW StartElevator(obj);
	else
		elevator = DBG_NEW Elevator(obj);


	// TODO: hack- fit offset to level elevator. need to find a better way
	switch (levelNumber)
	{
	case 2: elevator->_offsetY = -72; break;
	case 3:
	case 4: elevator->_offsetY = -80; break;
	case 7: elevator->_offsetY = 0; break;
	default: elevator->_offsetY = -64; break;
	}

	return elevator;
}


Elevator::Elevator(const WwdObject& obj)
	: BaseDynamicPlaneObject(obj), _arrivedToEdge(false),
	_minPos({ (float)obj.minX, (float)obj.minY }), _maxPos({ (float)obj.maxX, (float)obj.maxY }),
	_initialPos({ (float)obj.x, (float)obj.y }), _initialSpeed({}),
	_isOneWayElevator(contains(obj.logic, "OneWay")),
	_operateElevator(true), _offsetY(-64)
{
	_ani = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");

	if (obj.minX > 0 && obj.maxX > 0)
	{
		if (obj.speedX > 0)
		{
			speed.x = (float)obj.speedX;
		}
		else
		{
			speed.x = 125;
		}
	}
	if (obj.minY > 0 && obj.maxY > 0)
	{
		if (obj.speedY > 0)
		{
			speed.y = (float)obj.speedY;
		}
		else if (obj.speedX > 0)
		{
			speed.y = (float)obj.speedX;
		}
		else
		{
			speed.y = 125;
		}
	}

	switch (obj.direction)
	{
	case WwdObject::Direction_BottomLeft:	speed = { -abs(speed.x), abs(speed.y) }; break;
	case WwdObject::Direction_Down:			speed = { 0, abs(speed.y) }; break;
	case WwdObject::Direction_BottomRight:	speed = { abs(speed.x), abs(speed.y) }; break;
	case WwdObject::Direction_Left:			speed = { -abs(speed.x), 0 }; break;
	case WwdObject::Direction_NoMove:		speed = { 0, 0 }; break;
	case WwdObject::Direction_Right:		speed = { abs(speed.x), 0 }; break;
	case WwdObject::Direction_TopLeft:		speed = { -abs(speed.x), -abs(speed.y) }; break;
	case WwdObject::Direction_Up:			speed = { 0, -abs(speed.y) }; break;
	case WwdObject::Direction_TopRight:		speed = { abs(speed.x), -abs(speed.y) }; break;
	default: break;
	}

	speed.x /= 1000;
	speed.y /= 1000;

	myMemCpy(_initialSpeed, speed);
}
void Elevator::Logic(uint32_t elapsedTime)
{
	if (_isOneWayElevator && _arrivedToEdge)
	{
		return;
	}

	if ((speed.x < 0 && position.x < _minPos.x) || (speed.x > 0 && position.x > _maxPos.x))
	{
		speed.x = -speed.x;
		speed.y = -speed.y;
		_arrivedToEdge = true;
	}

	if ((speed.y < 0 && position.y < _minPos.y) || (speed.y > 0 && position.y > _maxPos.y))
	{
		speed.y = -speed.y;
		_arrivedToEdge = true;
	}

	mainLogic(elapsedTime);
}
void Elevator::mainLogic(uint32_t elapsedTime) // logic for every elevator
{
	const float deltaX = speed.x * elapsedTime, deltaY = speed.y * elapsedTime;

	position.x += deltaX;
	position.y += deltaY;

	if (player->elevator == this)
	{
		const Rectangle2D thisRc = GetRect(), playerRc = player->GetRect();

		// if no collision - disable the 'elevator mode' for player
		if (playerRc.right < thisRc.left || thisRc.right < playerRc.left)
		{
			player->elevator = nullptr;
		}
		else
		{
			player->position.x += deltaX;
			player->position.y = position.y + _offsetY;
		}
	}
	else
	{
		tryCatchPlayer();
	}
}
Rectangle2D Elevator::GetRect()
{
	Rectangle2D rc = BaseDynamicPlaneObject::GetRect();
	rc.top = rc.bottom - 20;
	return rc;
}
void Elevator::Reset()
{
	position = _initialPos;
	speed = _initialSpeed;
	_arrivedToEdge = false;
	if (_isOneWayElevator)
		_operateElevator = false;
}
bool Elevator::tryCatchPlayer()
{
	if (BaseDynamicPlaneObject::tryCatchPlayer())
	{
		player->elevator = this;
		return true;
	}
	return false;
}


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


StartElevator::StartElevator(const WwdObject& obj)
	: Elevator(obj)
{
	_operateElevator = false;
}
void StartElevator::Logic(uint32_t elapsedTime)
{
	if (player->elevator != this)
		_operateElevator = tryCatchPlayer(); // if CC left the elevator it will not keep moving (until he comes up again)
	if (_operateElevator)
		Elevator::Logic(elapsedTime);
}
void StartElevator::Reset()
{
	Elevator::Reset();
	_operateElevator = false;
}


PathElevator::PathElevator(const WwdObject& obj)
	: Elevator(obj), _totalSpeed(obj.speed ? (obj.speed / 1000.f) : 0.125f)
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
		case WwdObject::Direction_BottomLeft:	speed = { -_totalSpeed, _totalSpeed }; break;
		case WwdObject::Direction_Down:			speed = { 0, _totalSpeed }; break;
		case WwdObject::Direction_BottomRight:	speed = { _totalSpeed, _totalSpeed }; break;
		case WwdObject::Direction_Left:			speed = { -_totalSpeed, 0 }; break;
		case WwdObject::Direction_Right:		speed = { _totalSpeed, 0 }; break;
		case WwdObject::Direction_TopLeft:		speed = { -_totalSpeed, -_totalSpeed }; break;
		case WwdObject::Direction_Up:			speed = { 0, -_totalSpeed }; break;
		case WwdObject::Direction_TopRight:		speed = { _totalSpeed, -_totalSpeed }; break;
		default: speed = {}; break;
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
