#include "PathElevator.h"


PathElevator::PathElevator(const WwdObject& obj)
	: Elevator(obj), _totalSpeed(obj.speed ? (obj.speed / 1000.f) : 0.125f)
{
	if (obj.moveRect  .left  != 0) _paths.push_back({ obj.moveRect  .left , obj.moveRect  .top });
	if (obj.moveRect  .right != 0) _paths.push_back({ obj.moveRect  .right, obj.moveRect  .bottom });
	if (obj.hitRect   .left  != 0) _paths.push_back({ obj.hitRect   .left , obj.hitRect   .top });
	if (obj.hitRect   .right != 0) _paths.push_back({ obj.hitRect   .right, obj.hitRect   .bottom });
	if (obj.attackRect.left  != 0) _paths.push_back({ obj.attackRect.left , obj.attackRect.top });
	if (obj.attackRect.right != 0) _paths.push_back({ obj.attackRect.right, obj.attackRect.bottom });
	if (obj.clipRect  .left  != 0) _paths.push_back({ obj.clipRect  .left , obj.clipRect  .top });
	if (obj.clipRect  .right != 0) _paths.push_back({ obj.clipRect  .right, obj.clipRect  .bottom });

	if (_paths.size() == 0) throw Exception("PathElevator - no paths");

	_ani = AssetsManager::getAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet));

	position = _initialPos;
	_pathIdx = -1;

	_timer = DBG_NEW Timer((int)(_paths[0].second / _totalSpeed), bind(&PathElevator::changeDirection, this));
	addTimer(_timer);
}
PathElevator::~PathElevator() {
	delete _timer;
}

void PathElevator::Logic(uint32_t elapsedTime)
{
	mainLogic(elapsedTime);
}

void PathElevator::Reset()
{
	position = _initialPos;
	_pathIdx = -1;
	_timer->reset();
}

void PathElevator::changeDirection()
{
	_pathIdx = (_pathIdx + 1) % _paths.size();
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

	_timer->reset((int)(_paths[_pathIdx].second / _totalSpeed));
}
