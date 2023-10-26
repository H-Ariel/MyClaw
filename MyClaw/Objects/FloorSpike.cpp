#include "FloorSpike.h"
#include "../Assets-Managers/AssetsManager.h"


// TODO: This functions are same to TogglePeg. maybe we can combine them

// FloorSpike ctor. The purpose of `doFloorSpikeCtor` flag is to save time in children classes
FloorSpike::FloorSpike(const WwdObject& obj, bool doFloorSpikeCtor)
	: BaseDamageObject(obj, 10), _framesAmount(0), _startTimeDelay(0)
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
	vector<Animation::FrameData*> images = AssetsManager::createAnimationFromDirectory(imageSetPath, 125, false)->getImagesList();
	vector<Animation::FrameData*> disappearImages = AssetsManager::createAnimationFromDirectory(imageSetPath, 125, true)->getImagesList();


	myMemCpy(images.back()->duration, uint32_t((obj.speedX > 0) ? obj.speedX : 1500));
	myMemCpy(disappearImages.back()->duration, uint32_t((obj.speedY > 0) ? obj.speedY : 1500));

	images.insert(images.end(), disappearImages.begin(), disappearImages.end());

	_ani = allocNewSharedPtr<Animation>(images);
	_framesAmount = images.size();

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
	size_t idx = _ani->getFrameNumber() * 4;
	return (_framesAmount <= idx && idx <= _framesAmount * 3);
}


// TODO: spin animation is not implemented yet
SawBlade::SawBlade(const WwdObject& obj)
	: FloorSpike(obj, false)
{
	/*
	// that written in FloorSpike constructor
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
	*/

	vector<Animation::FrameData*> appearImages = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_UP"))->getImagesList();
//	vector<Animation::FrameData*> waitImages = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_SPIN"))->getImagesList();
	vector<Animation::FrameData*> disappearImages = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_SAWBLADE_DOWN"))->getImagesList();
	vector<Animation::FrameData*> images;

	myMemCpy(appearImages.back()->duration, uint32_t((obj.speedX > 0) ? obj.speedX : 1500));
	myMemCpy(disappearImages.back()->duration, uint32_t((obj.speedY > 0) ? obj.speedY : 1500));

	images.insert(images.end(), appearImages.begin(), appearImages.end());
//	images.insert(images.end(), waitImages.begin(), waitImages.end());
	images.insert(images.end(), disappearImages.begin(), disappearImages.end());

	_ani = allocNewSharedPtr<Animation>(images);
	_framesAmount = images.size();

	setObjectRectangle();
}

LavaGeyser::LavaGeyser(const WwdObject& obj)
	: FloorSpike(obj, false)
{
	vector<Animation::FrameData*> images = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_LAVAGEYSER"), 50, false)->getImagesList();
	myMemCpy(images.back()->duration, uint32_t((obj.speed > 0) ? obj.speed : 500));
	_ani = allocNewSharedPtr<Animation>(images);
	_framesAmount = images.size();
	setObjectRectangle();
}
