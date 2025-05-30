#include "FloorSpike.h"
#include "../GlobalObjects.h"
#include "../CheatsManager.h"


// TODO: these ctor & logic same to TogglePeg, maybe can combine them (or part of them)

// FloorSpike ctor. The purpose of `doFloorSpikeCtor` flag is to save time in children classes. TODO: check obj.logic \O/
FloorSpike::FloorSpike(const WwdObject& obj, bool doFloorSpikeCtor)
	: BaseDamageObject(obj, 10), _startTimeDelay(0)
{
	int startTimeDelay = 0;
	if (obj.speed > 0)
	{
		startTimeDelay = obj.speed;
	}
	else
	{
		switch (obj.logic.back())
		{
		case '2': startTimeDelay = 750; break;
		case '3': startTimeDelay = 1500; break;
		case '4': startTimeDelay = 2250; break;
		}
	}

	myMemCpy(_startTimeDelay, startTimeDelay);
	Delay(_startTimeDelay);

	if (!doFloorSpikeCtor) return;

	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));
	vector<UIAnimation::FrameData*> appearImages = AssetsManager::createAnimationFromDirectory(imageSetPath)->getFramesList();
	vector<UIAnimation::FrameData*> disappearImages = AssetsManager::createAnimationFromDirectory(imageSetPath, true)->getFramesList();

	myMemCpy(appearImages.back()->duration, uint32_t((obj.speedX > 0) ? obj.speedX : 1500));
	myMemCpy(disappearImages.back()->duration, uint32_t((obj.speedY > 0) ? obj.speedY : 1500));

	_ani = make_shared<UIAnimation>(appearImages + disappearImages);

	setObjectRectangle();
}
void FloorSpike::Logic(uint32_t elapsedTime)
{
	// this `if` block has copied from TogglePeg::Logic ... is it bad ?
	if (GO::cheats->isEasyMode()) { // stop animation after it finished (stay at 'off' state)
		// Checking whether we are in the 1st or 4th quarter of the frames' sequence
		float progress = _ani->getFramesProgress();
		if (progress <= 0.25f || 0.75 <= progress)
		{
			if (_ani->isFinishAnimation())
				_ani->updateFrames = false;
		}

		// TODO (?): find algorithm and use `enterEasyMode`
	}

	_ani->Logic(elapsedTime);
}
bool FloorSpike::isDamage() const
{
	// catch player between the second and third quarters
	float progress = _ani->getFramesProgress();
	return 0.25f <= progress && progress <= 0.75;
}
void FloorSpike::exitEasyMode() {
	_ani->updateFrames = true;
	Delay(_startTimeDelay);
}

SawBlade::SawBlade(const WwdObject& obj)
	: FloorSpike(obj, false)
{
	shared_ptr<UIAnimation> spinAni = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_SPIN"));
	vector<UIAnimation::FrameData*> appearImages = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_UP"))->getFramesList();
	vector<UIAnimation::FrameData*> disappearImages = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_DOWN"))->getFramesList();
	vector<UIAnimation::FrameData*> waitImages;

	int spinTotalDuration = (int)spinAni->getTotalDuration();

	for (int timeUp = (obj.speedX > 0) ? obj.speedX : 1500; timeUp > 0; timeUp -= spinTotalDuration)
	{
		waitImages += spinAni->getFramesList();
	}
	
	myMemCpy(disappearImages.back()->duration, uint32_t((obj.speedY > 0) ? obj.speedY : 1500));
	
	_ani = make_shared<UIAnimation>(appearImages + waitImages + disappearImages);

	setObjectRectangle();
}
bool SawBlade::isDamage() const
{
	size_t idx = _ani->getFrameNumber(), framesAmount = _ani->getFramesCount();
	return idx != 0 && idx != framesAmount - 1;
}


LavaGeyser::LavaGeyser(const WwdObject& obj)
	: FloorSpike(obj, false)
{
	vector<UIAnimation::FrameData*> images = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_LAVAGEYSER"), false,50)->getFramesList();
	myMemCpy(images.back()->duration, uint32_t((obj.speed > 0) ? obj.speed : 500));
	_ani = make_shared<UIAnimation>(images);
	setObjectRectangle();
}
