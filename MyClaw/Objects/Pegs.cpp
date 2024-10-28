#include "Pegs.h"
#include "../CheatsManager.h"


static bool canCatchPlayer(UIAnimation* ani) {
	// catch player between the 1st and 4th quarters
	float progress = ani->getFramesProgress();
	return progress <= 0.25f || 0.75 <= progress;
}

TogglePeg::TogglePeg(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _startTimeDelay(0)
{
	int startTimeDelay = 0;

	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));
	bool isAlwaysOn = false;

	if (obj.speed > 0)
	{
		startTimeDelay = obj.speed;
	}
	else
	{
		if (startsWith(obj.logic, "SteppingStone"))
		{
			switch (obj.logic.back())
			{
			case '2': startTimeDelay = 500; break;
			case '3': startTimeDelay = 1000; break;
			case '4': startTimeDelay = 1500; break;
			}

			if (obj.speedX <= 0) myMemCpy(obj.speedX, 150);
			if (obj.speedY <= 0) myMemCpy(obj.speedY, 500);
			myMemCpy(obj.smarts, 0);
		}
		else // regular TogglePeg
		{
			switch (obj.logic.back())
			{
			case '2': startTimeDelay = 750; break;
			case '3': startTimeDelay = 1500; break;
			case '4': startTimeDelay = 2250; break;
			}
		}
	}

	myMemCpy(_startTimeDelay, startTimeDelay);
	Delay(_startTimeDelay);

	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(imageSetPath, true)->getImagesList();

	myMemCpy(images.back()->duration, uint32_t(obj.speedX > 0 ? obj.speedX : 1500));

	if (obj.smarts & 0x1)
	{
		isAlwaysOn = true;
		// save only last frame
		for_each(images.begin(), images.end() - 1, [](UIAnimation::FrameData* f) { delete f; });
		images.erase(images.begin(), images.end() - 1);
	}
	else
	{
		// combine images in straight and reverse order for full animation
		vector<UIAnimation::FrameData*> appearImages = AssetsManager::createAnimationFromDirectory(imageSetPath)->getImagesList();
		myMemCpy(appearImages.back()->duration, uint32_t(obj.speedY > 0 ? obj.speedY : 1500)); // last frame 'waits' (easy to implementation)
		images = appearImages + images;
	}

	_ani = make_shared<UIAnimation>(images);
	_ani->updateFrames = !isAlwaysOn;

	setObjectRectangle();

	// this used in level 8
	if (obj.imageSet == "LEVEL_CANNON2")
	{
		myMemCpy(_objRc.top, _objRc.top + 32);
	}

	drawZ = DefaultZCoord::Pegs;
}
void TogglePeg::Logic(uint32_t elapsedTime)
{
	if (cheats->isEasyMode()) { // stop animation after it finished (stay at 'on' state)
		if (canCatchPlayer(_ani.get()))
		{
			if (_ani->isFinishAnimation())
				_ani->updateFrames = false;
			tryCatchPlayer();
		}

		// TODO (?): find algorithm and use `enterEasyMode`
	}

	_ani->Logic(elapsedTime);
	if (canCatchPlayer(_ani.get()))
		tryCatchPlayer();
}
void TogglePeg::enterEasyMode() { }
void TogglePeg::exitEasyMode() {
	_ani->updateFrames = true;
	Delay(_startTimeDelay);
}

StartSteppingStone::StartSteppingStone(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));

	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(imageSetPath, true)->getImagesList();
	myMemCpy(images.back()->duration, uint32_t(obj.speedX > 0 ? obj.speedX : 1000));
	vector<UIAnimation::FrameData*> appearImages = AssetsManager::createAnimationFromDirectory(imageSetPath, false)->getImagesList();
	myMemCpy(appearImages.back()->duration, uint32_t(obj.speedY > 0 ? obj.speedY : 2000));
	images = appearImages + images;

	_ani = make_shared<UIAnimation>(images);

	setObjectRectangle();
}
void StartSteppingStone::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);

	if (_ani->updateFrames)
	{
		if (canCatchPlayer(_ani.get()))
			tryCatchPlayer();

		if (_ani->getFrameNumber() == 0)
			_ani->updateFrames = false;
	}
	else
	{
		if (tryCatchPlayer())
		{
			_ani->updateFrames = true;
		}
	}
}

CrumblingPeg::CrumblingPeg(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet))->getImagesList();
	myMemCpy(images[0]->duration, (uint32_t)obj.counter);
	_ani = make_shared<UIAnimation>(images);
	_ani->position = position;
	Reset();
	setObjectRectangle();
}
void CrumblingPeg::Logic(uint32_t elapsedTime)
{
	if (_ani->isFinishAnimation())
		return; // the object is not used

	if (!_ani->isPassedHalf() && tryCatchPlayer())
		_ani->updateFrames = !cheats->isEasyMode();

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
	myMemCpy(_objRc.top, _objRc.top + topOffset);
}
