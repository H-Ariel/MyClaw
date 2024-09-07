#include "Pegs.h"
#include "../CheatsManager.h"


void pegTryCatchPlayer(BaseStaticPlaneObject* peg, shared_ptr<UIAnimation> ani)
{
	// catch player between the second and third quarters
	size_t idx = ani->getFrameNumber() * 4, framesAmount = ani->getImagesCount();
	if (idx <= framesAmount || framesAmount * 3 <= idx)
	{
		peg->tryCatchPlayer();
	}
}


TogglePeg::TogglePeg(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _startTimeDelay(0)
{
	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));
	bool isAlwaysOn = false;

	if (obj.speed > 0)
	{
		_startTimeDelay = obj.speed;
	}
	else
	{
		if (startsWith(obj.logic, "SteppingStone"))
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

	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(imageSetPath, true)->getImagesList();

	myMemCpy(images.back()->duration, uint32_t(obj.speedX > 0 ? obj.speedX : 1500));

	if (obj.smarts & 0x1)
	{
		isAlwaysOn = true;
		for_each(images.begin(), images.end() - 1, [](UIAnimation::FrameData* f) { delete f; });
		images.erase(images.begin(), images.end() - 1);
	}
	else
	{
		vector<UIAnimation::FrameData*> appearImages = AssetsManager::createAnimationFromDirectory(imageSetPath)->getImagesList();
		myMemCpy(appearImages.back()->duration, uint32_t(obj.speedY > 0 ? obj.speedY : 1500));
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
	if (_startTimeDelay > 0)
	{
		_startTimeDelay -= elapsedTime;
		return;
	}

	if (cheats->isEasyMode()) {
 		size_t idx = _ani->getFrameNumber() * 4, framesAmount = _ani->getImagesCount();
 		if (idx <= framesAmount || framesAmount * 3 <= idx)
 		{
			if(_ani->isFinishAnimation())
			_ani->updateFrames = false;
 			tryCatchPlayer();
 		}

		// TODO: when enable easy-mode and then desable it, the pegs will not work
	}

	_ani->Logic(elapsedTime);
	pegTryCatchPlayer(this, _ani);
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
		pegTryCatchPlayer(this, _ani);

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
