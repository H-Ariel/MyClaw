#include "FloorSpike.h"
#include "../Player.h"
#include "../AssetsManager.h"


// This functions are same to TogglePeg. maybe we can combine them

FloorSpike::FloorSpike(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _state(States::WaitAppear),
	_totalTime(0), _startTimeDelay(0), _timeOn(0), _timeOff(0)
{
	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));
	_aniAppear = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, false);
	_aniDisappear = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, true);

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

	setObjectRectangle();
}

void FloorSpike::Logic(uint32_t elapsedTime)
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
			_ani->reset();
			_ani->loopAni = false;
		}
		break;
	}

	_ani->Logic(elapsedTime);
}

int FloorSpike::getDamage() const
{
	if ((_state == States::Disappear && !_ani->isPassedHalf()) ||
		(_state == States::Appear && _ani->isPassedHalf()) ||
		_state == States::WaitAppear) return 10;
	return 0;
}


SawBlade::SawBlade(const WwdObject& obj, Player* player)
	: FloorSpike(obj, player)
{
	_aniAppear = AssetsManager::loadCopyAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_UP"));
	_aniDisappear = AssetsManager::loadCopyAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_DOWN"));
	_aniWait = AssetsManager::loadCopyAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_SPIN"));
	_ani = _aniDisappear;
	_ani->updateFrames = false;
}

void SawBlade::Logic(uint32_t elapsedTime)
{
	if (_state == States::WaitAppear)
		_ani = _aniWait;

	FloorSpike::Logic(elapsedTime);
}
