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
		if (obj.logic == "StartSteppingStone")
		{
			if (obj.speedX <= 0) myMemCpy(obj.speedX, 1000);
			if (obj.speedY <= 0) myMemCpy(obj.speedY, 2000);
			myMemCpy(obj.smarts, 0);
		}
		else if (startsWith(obj.logic, "SteppingStone"))
		{
			switch (obj.logic.back())
			{
			case '2': _startTimeDelay = 500; break;
			case '3': _startTimeDelay = 1000; break;
			case '4': _startTimeDelay = 1500; break;
			}

			if (obj.speedX <= 0) myMemCpy(obj.speedX, 150);
			if (obj.speedY <= 0) myMemCpy(obj.speedY, 500);
			myMemCpy(obj.smarts, 0);
		}
		else // regular TogglePeg
		{
			switch (obj.logic.back())
			{
			case '2': _startTimeDelay = 750; break;
			case '3': _startTimeDelay = 1500; break;
			case '4': _startTimeDelay = 2250; break;
			}
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

	size_t idx = _ani->getFrameNumber() * 4;
	if (idx <= _framesAmount || _framesAmount * 3 <= idx)
	{
		tryCatchPlayer();
	}
}

CrumblingPeg::CrumblingPeg(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	vector<Animation::FrameData*> images = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 125, false)->getImagesList();
	myMemCpy(images[0]->duration, (uint32_t)obj.counter);
	_ani = allocNewSharedPtr<Animation>(images);
	_ani->position = position;
	Reset();
	setObjectRectangle();
}
void CrumblingPeg::Logic(uint32_t elapsedTime)
{
	if (_ani->isFinishAnimation())
		return; // the object is not used

	if (!_ani->isPassedHalf() && tryCatchPlayer())
		_ani->updateFrames = true;

	_ani->Logic(elapsedTime);
}
void CrumblingPeg::Draw()
{
	if (!_ani->isFinishAnimation())
		_ani->Draw();
}
void CrumblingPeg::Reset()
{
	_ani->reset();
	_ani->updateFrames = false;
	_ani->loopAni = false;
}

BreakPlank::BreakPlank(const WwdObject& obj, float topOffset)
	: CrumblingPeg(obj)
{
	Rectangle2D rc(_objRc);
	rc.top += topOffset;
	myMemCpy(_objRc, rc);
}
