#include "FloorSpike.h"


// FloorSpike ctor. The purpose of `doFloorSpikeCtor` flag is to save time in children classes
FloorSpike::FloorSpike(const WwdObject& obj, bool doFloorSpikeCtor)
	: BaseDamageObject(obj, 10), _startTimeDelay(0)
{
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

	if (!doFloorSpikeCtor) return;

	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));
	vector<UIAnimation::FrameData*> appearImages = AssetsManager::createAnimationFromDirectory(imageSetPath)->getImagesList();
	vector<UIAnimation::FrameData*> disappearImages = AssetsManager::createAnimationFromDirectory(imageSetPath, true)->getImagesList();

	myMemCpy(appearImages.back()->duration, uint32_t((obj.speedX > 0) ? obj.speedX : 1500));
	myMemCpy(disappearImages.back()->duration, uint32_t((obj.speedY > 0) ? obj.speedY : 1500));

	_ani = make_shared<UIAnimation>(appearImages + disappearImages);

	setObjectRectangle();
}
void FloorSpike::Logic(uint32_t elapsedTime)
{
	if (_startTimeDelay > 0)
	{
		_startTimeDelay -= elapsedTime;
		return;
	}

	_ani->Logic(elapsedTime);
}
bool FloorSpike::isDamage() const
{
	size_t idx = _ani->getFrameNumber() * 4, framesAmount = _ani->getImagesCount();
	return (framesAmount <= idx && idx <= framesAmount * 3);
}


SawBlade::SawBlade(const WwdObject& obj)
	: FloorSpike(obj, false)
{
	shared_ptr<UIAnimation> spinAni = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_SPIN"));
	vector<UIAnimation::FrameData*> appearImages = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_UP"))->getImagesList();
	vector<UIAnimation::FrameData*> disappearImages = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_DOWN"))->getImagesList();

	vector<UIAnimation::FrameData*> waitImages;
	for (int32_t timeUp = (obj.speedX > 0) ? obj.speedX : 1500; timeUp > 0;)
	{
		waitImages += spinAni->getImagesList();
		timeUp -= (int32_t)spinAni->getTotalDuration();
	}
	
	myMemCpy(disappearImages.back()->duration, uint32_t((obj.speedY > 0) ? obj.speedY : 1500));
	
	_ani = make_shared<UIAnimation>(appearImages + waitImages + disappearImages);

	setObjectRectangle();
}
bool SawBlade::isDamage() const
{
	size_t idx = _ani->getFrameNumber(), framesAmount = _ani->getImagesCount();
	return idx != 0 && idx != framesAmount - 1;
}


LavaGeyser::LavaGeyser(const WwdObject& obj)
	: FloorSpike(obj, false)
{
	vector<UIAnimation::FrameData*> images = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_LAVAGEYSER"), false,50)->getImagesList();
	myMemCpy(images.back()->duration, uint32_t((obj.speed > 0) ? obj.speed : 500));
	_ani = make_shared<UIAnimation>(images);
	setObjectRectangle();
}
