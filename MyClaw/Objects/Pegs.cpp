#include "Pegs.h"
#include "../AssetsManager.h"


// I improved the code, but it affect the memory usage.
// TODO: find way to use the improved code and not affect the memory usage.
// (the problem is duplicate images and animation in the memory)


TogglePeg::TogglePeg(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _startTimeDelay(0), _isAlwaysOn(false)
{
	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));

	if (obj.speed > 0)
	{
		_startTimeDelay = obj.speed;
	}
	else
	{
		switch (obj.logic.back())
		{
		case '2': _startTimeDelay = 750; break;
		case '3': _startTimeDelay = 1500; break;
		case '4': _startTimeDelay = 2250; break;
		}
	}

	vector<Animation::FrameData*> appearImages = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, true)->getImagesList();
	vector<Animation::FrameData*> images;

	myMemCpy(appearImages.back()->duration, uint32_t(obj.speedX > 0 ? obj.speedX : 1500));

	if (obj.smarts & 0x1)
	{
		_isAlwaysOn = true;

		images.push_back(appearImages.back());
		appearImages.erase(appearImages.end() - 1);
		for (auto i : appearImages) delete i;
	}
	else
	{
		uint32_t timeOff = obj.speedY > 0 ? obj.speedY : 1500;

		vector<Animation::FrameData*> disappearImages = AssetsManager::createCopyAnimationFromDirectory(imageSetPath, 125, false)->getImagesList();
		myMemCpy(disappearImages.back()->duration, timeOff);

		images = disappearImages;
		images.insert(images.end(), appearImages.begin(), appearImages.end());
	}

	_ani = allocNewSharedPtr<Animation>(images);
	_framesAmount = images.size();

	setObjectRectangle();

	//TODO: used in level 8
	if (obj.imageSet == "LEVEL_CANNON2")
	{
		myMemCpy(_objRc.top, _objRc.top + 32);
	}
}
void TogglePeg::Logic(uint32_t elapsedTime)
{
	if (_isAlwaysOn)
	{
		tryCatchPlayer();
		return;
	}

	if (_startTimeDelay > 0)
	{
		_startTimeDelay -= elapsedTime;
		return;
	}

	_ani->Logic(elapsedTime);

	size_t idx = _ani->getFrameNumber();

	size_t quarter1 = _framesAmount / 4; // 0 - 1/4
//	size_t quarter2 = _framesAmount / 2; // 1/4 - 1/2
	size_t quarter3 = 3 * _framesAmount / 4; // 1/2 - 3/4
//	size_t quarter4 = _framesAmount; // 3/4 - 1

	if (idx <= quarter1 || quarter3 <= idx)
	{
		tryCatchPlayer();
	}
}

CrumblingPeg::CrumblingPeg(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _delayTime(obj.counter)
{
	vector<Animation::FrameData*> images = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 125, false)->getImagesList();
	myMemCpy(images.front()->duration, (uint32_t)_delayTime);
	_ani = allocNewSharedPtr<Animation>(images);
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
			_state = States::Crumbling;
			_ani->updateFrames = true;
		}

	if (_state == States::Crumbling)
	{
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
	_state = States::Appear;
	_used = true;
	_draw = true;
}

BreakPlank::BreakPlank(const WwdObject& obj, float topOffset)
	: CrumblingPeg(obj)
{
	Rectangle2D rc(_objRc);
	rc.top += topOffset;
	myMemCpy(_objRc, rc);
}
