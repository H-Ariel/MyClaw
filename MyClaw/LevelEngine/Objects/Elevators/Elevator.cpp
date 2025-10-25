#include "../../GlobalObjects.h"
#include "../Player/Player.h"

// The following classes indicate precisely the type of elevator:
// TriggerElevator: the elevator starts move when the player touches it (and never stops)
// StartElevator: the elevator starts move when the player touches it (and stops when he leaves it)
// PathElevator: the elevator moves along a path (defined in the logic field of the object)
#include "PathElevator.h"
#include "StartElevator.h"
#include "TriggerElevator.h"



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
	: BaseDynamicPlaneObject(obj, true), _arrivedToEdge(false),
	_minPos({ (float)obj.minX, (float)obj.minY }), _maxPos({ (float)obj.maxX, (float)obj.maxY }),
	_initialPos({ (float)obj.x, (float)obj.y }), _initialSpeed({}),
	_isOneWayElevator(contains(obj.logic, "OneWay")),
	_operateElevator(true), _offsetY(-64)
{
	_ani = AssetsManager::getAnimationFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");

	if (obj.minX > 0 && obj.maxX > 0)
	{
		speed.x = (obj.speedX > 0) ? (float)obj.speedX : 125;
	}
	if (obj.minY > 0 && obj.maxY > 0)
	{
		speed.y = (obj.speedY > 0) ? (float)obj.speedY
			: (obj.speedX > 0) ? (float)obj.speedX
			: 125;
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

	if (GO::player->elevator == this)
	{
		const Rectangle2D thisRc = GetRect(), playerRc = GO::getPlayerRect();

		// if no collision - disable the 'elevator mode' for player
		if (playerRc.right < thisRc.left || thisRc.right < playerRc.left)
		{
			GO::player->elevator = nullptr;
		}
		else
		{
			GO::getPlayerPosition().x += deltaX;
			GO::getPlayerPosition().y = position.y + _offsetY;
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
		GO::player->elevator = this;
		return true;
	}
	return false;
}
