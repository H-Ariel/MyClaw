#include "AdaptiveImage.h"
#include "AssetsManagers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


AdaptiveImage::AdaptiveImage(const string& imagePath, D2D1_POINT_2F posRatio, BackgroundImage* bgImg)
	: UIBaseImage(*AssetsManager::loadImage(imagePath)), ScreenRelative(posRatio, bgImg)
{
	_sizeRatio.width = WindowManager::DEFAULT_WINDOW_SIZE.width / size.width;
	_sizeRatio.height = WindowManager::DEFAULT_WINDOW_SIZE.height / size.height;

	Logic(0); // just to update the size and position
}

void AdaptiveImage::Logic(uint32_t)
{
	updateSizeAndPosition(this->size, this->position);
}
