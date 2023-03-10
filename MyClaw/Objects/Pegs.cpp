#include "Pegs.h"
#include "../AssetsManager.h"
#include "../WindowManager.h"


TogglePeg::TogglePeg(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _state(States::WaitAppear),
	_totalTime(0), _startTimeDelay(0), _timeOn(0), _timeOff(0)
{
	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));
	_aniAppear = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, true);
	_aniDisappear = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, false);

	_ani = _aniDisappear;
	_ani->updateFrames = false;

	if (obj.speed > 0)
	{
		_startTimeDelay = obj.speed;
	}
	else
	{
		switch (obj.logic[obj.logic.length() - 1])
		{
		case '2': _startTimeDelay = 750; break;
		case '3': _startTimeDelay = 1500; break;
		case '4': _startTimeDelay = 2250; break;
		}
	}

	if (obj.speedX > 0)
	{
		_timeOn = obj.speedX;
	}
	else
	{
		_timeOn = 1500;
	}
	if (obj.speedY > 0)
	{
		_timeOff = obj.speedY;
	}
	else
	{
		_timeOff = 1500;
	}

	if (obj.smarts & 0x1)
	{
		_timeOff = 0; // it's always on
	}

	setObjectRectangle();
}
void TogglePeg::Logic(uint32_t elapsedTime)
{
	if (_startTimeDelay > 0)
	{
		_startTimeDelay -= elapsedTime;
		return;
	}

	_totalTime += elapsedTime;

	switch (_state)
	{
	case States::Appear:
		if (_ani->isFinishAnimation())
		{
			_state = States::WaitAppear;
			_totalTime = 0;
			_ani->updateFrames = false;
		}
		break;

	case States::WaitAppear:
		if (_totalTime >= _timeOn && _timeOff > 0)
		{
			_state = States::Disappear;
			_ani = _aniDisappear;
			setObjectRectangle();
			_ani->reset();
			_ani->loopAni = false;
		}
		break;

	case States::Disappear:
		if (_ani->isFinishAnimation())
		{
			_state = States::WaitDisappear;
			_totalTime = 0;
			_ani->updateFrames = false;
		}
		break;

	case States::WaitDisappear:
		if (_totalTime >= _timeOff)
		{
			_state = States::Appear;
			_ani = _aniAppear;
			setObjectRectangle();
			_ani->reset();
			_ani->loopAni = false;
		}
		break;
	}

	_ani->Logic(elapsedTime);

	if ((_state == States::Appear && _ani->isPassedHalf()) || 
		(_state == States::Disappear && !_ani->isPassedHalf()) || 
		_state == States::WaitAppear)
	{
		tryCatchPlayer();
	}
}

CrumblingPeg::CrumblingPeg(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _delayTime(obj.counter)
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 125, false);
	_ani->position = position;
	Reset();
	setObjectRectangle();
}
void CrumblingPeg::Logic(uint32_t elapsedTime)
{
	if (!_draw) return;

	if (_used)
		if (tryCatchPlayer())
		{
			_state = States::Wait;
		}

	if (_state == States::Wait)
	{
		_timeCounter += elapsedTime;
		if (_timeCounter >= _delayTime)
		{
			_state = States::Crumbling;
			_ani->updateFrames = true;
		}
	}

	if (_state == States::Crumbling)
	{
		// if it's finished crumbling we don't need that object anymore
		_draw = !_ani->isFinishAnimation();
		_used = !_ani->isPassedHalf();
		_ani->Logic(elapsedTime);
	}
}
void CrumblingPeg::Draw()
{
	if (_draw)
		_ani->Draw();
}
void CrumblingPeg::Reset()
{
	_ani->reset();
	_ani->updateFrames = false;
	_ani->loopAni = false;
	_timeCounter = 0;
	_state = States::Appear;
	_used = true;
	_draw = true;
}

BreakPlank::BreakPlank(const WwdObject& obj, Player* player, WwdRect tileRc)
	: CrumblingPeg(obj, player)
{
	D2D1_RECT_F rc = _objRc;
	rc.top += tileRc.top;
	myMemCpy(_objRc, rc);
}
