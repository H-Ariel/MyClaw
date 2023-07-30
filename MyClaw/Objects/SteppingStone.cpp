#include "SteppingStone.h"
#include "../AssetsManager.h"


// TODO: looks same to TogglePeg ... (NOTE: the times in the ctor are different!)
// TODO: more over, SteppingStone same to StartSteppingStone !

SteppingStone::SteppingStone(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _state(States::WaitAppear),
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
		switch (obj.logic.back())
		{
		case '2': _startTimeDelay = 500; break;
		case '3': _startTimeDelay = 1000; break;
		case '4': _startTimeDelay = 1500; break;
		}
	}

	if (obj.speedX > 0)
	{
		_timeOn = obj.speedX;
	}
	else
	{
		_timeOn = 250;
	}
	if (obj.speedY > 0)
	{
		_timeOff = obj.speedY;
	}
	else
	{
		_timeOff = 500;
	}


	setObjectRectangle();
}
void SteppingStone::Logic(uint32_t elapsedTime)
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


StartSteppingStone::StartSteppingStone(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _state(States::WaitAppear),
	_totalTime(0), _timeOn(0), _timeOff(0)
{
	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));
	_aniAppear = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, true);
	_aniDisappear = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, false);

	_ani = _aniDisappear;
	_ani->updateFrames = false;

	if (obj.speedX > 0)
	{
		_timeOn = obj.speedX;
	}
	else
	{
		_timeOn = 1000;
	}
	if (obj.speedY > 0)
	{
		_timeOff = obj.speedY;
	}
	else
	{
		_timeOff = 2000;
	}


	setObjectRectangle();
}
void StartSteppingStone::Logic(uint32_t elapsedTime)
{
	// TODO: I think that some thing wrong in this method... never use copy-paste without understanding the code!

	_totalTime += elapsedTime;

	switch (_state)
	{
	case States::Appear:
		if (tryCatchPlayer())
		{
			_state = States::WaitAppear;
			_totalTime = 0;
			_ani->updateFrames = false;
		}
		break;

	case States::WaitAppear:
		if (_totalTime >= _timeOn)
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
