#include "OneTimeAnimation.h"


OneTimeAnimation::OneTimeAnimation(D2D1_POINT_2F pos, shared_ptr<UIAnimation> ani, bool removeAtEnd)
	: BasePlaneObject({}, true), _removeAtEnd(removeAtEnd)
{
	drawZ = DefaultZCoord::Characters + 2; // always cover characters
	_ani = ani;
	_ani->loopAni = false;
	_ani->position = pos;
}
OneTimeAnimation::OneTimeAnimation(D2D1_POINT_2F pos, const string& aniPath, const string& imageSet, bool removeAtEnd)
	: OneTimeAnimation(pos, AssetsManager::loadCopyAnimation(
		PathManager::getAnimationPath(aniPath), PathManager::getImageSetPath(imageSet)), removeAtEnd)
{
}
void OneTimeAnimation::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);
	if (_removeAtEnd)
		removeObject = _ani->isFinishAnimation();
}
void OneTimeAnimation::Draw()
{
	_ani->Draw();
}
