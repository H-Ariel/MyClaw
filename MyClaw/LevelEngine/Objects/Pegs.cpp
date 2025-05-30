#include "Pegs.h"
#include "../GlobalObjects.h"
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

	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(imageSetPath, true)->getFramesList();

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
		vector<UIAnimation::FrameData*> appearImages = AssetsManager::createAnimationFromDirectory(imageSetPath)->getFramesList();
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
	if (GO::cheats->isEasyMode()) { // stop animation after it finished (stay at 'on' state)
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
void TogglePeg::exitEasyMode() {
	_ani->updateFrames = true;
	Delay(_startTimeDelay);
}

StartSteppingStone::StartSteppingStone(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	const string imageSetPath(PathManager::getImageSetPath(obj.imageSet));

	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(imageSetPath, true)->getFramesList();
	myMemCpy(images.back()->duration, uint32_t(obj.speedX > 0 ? obj.speedX : 1000));
	vector<UIAnimation::FrameData*> appearImages = AssetsManager::createAnimationFromDirectory(imageSetPath, false)->getFramesList();
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
	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet))->getFramesList();
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
		_ani->updateFrames = !GO::cheats->isEasyMode();

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
	: BaseStaticPlaneObject(obj), _topOffset(topOffset)
{
	vector<UIAnimation::FrameData*> images = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet))->getFramesList();
	myMemCpy(images[0]->duration, (uint32_t)obj.counter);
	_ani = make_shared<UIAnimation>(images);

	_planks.reserve(obj.width);
	float x = (float)obj.x;
	for (int i = 0; i < obj.width; i++) {
		_planks.push_back(_ani->getCopy());
		_planks.back()->position = { x, (float)obj.y };
		x += TILE_SIZE;
	}

	Reset();
}
void BreakPlank::Logic(uint32_t elapsedTime) {
	// as soon as CC touches the plank, it begins to crumble
	if (tryCatchPlayer()) {
		if (activeIdx == 0) {
			_planks[0]->updateFrames = !GO::cheats->isEasyMode();;
		}
	}

	// executes the logic for the current animation, and when it ends runs the one after it
	for (int i = 0; i < _planks.size(); i++) {
		_planks[i]->Logic(elapsedTime);
		if (i == activeIdx && _planks[i]->getFramesProgress() >= 0.3f) {
			activeIdx += 1;
			if (activeIdx < _planks.size())
				_planks[activeIdx]->updateFrames = !GO::cheats->isEasyMode();
			myMemCpy(_objRc.left, _objRc.left + TILE_SIZE);
		}
	}
}
void BreakPlank::Draw() {
	for (auto& a : _planks)
		a->Draw();
}
void BreakPlank::Reset() {
	for (auto& a : _planks) {
		a->reset();
		a->updateFrames = false;
		a->loopAni = false;
	}
	activeIdx = 0;

	setObjectRectangle();
	myMemCpy(_objRc.right, _planks.back()->position.x);
	myMemCpy(_objRc.top, _objRc.top + _topOffset);
}
