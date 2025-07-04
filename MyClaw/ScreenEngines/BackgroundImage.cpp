#include "BackgroundImage.h"
#include "AssetsManagers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


BackgroundImage::BackgroundImage(const string& imagePath)
	: UIBaseImage(*AssetsManager::loadImage(imagePath)), _imgRatio(size.width / size.height) {}
void BackgroundImage::Logic(uint32_t)
{
	// change image size according to window size:

	D2D1_SIZE_F wndSz = WindowManager::getRealSize();

	if (wndSz.width > wndSz.height)
	{
		size.height = wndSz.height;
		size.width = size.height * _imgRatio;
	}
	else if (wndSz.width < wndSz.height)
	{
		size.width = wndSz.width;
		size.height = size.width / _imgRatio;
	}

	position.x = wndSz.width / 2;
	position.y = wndSz.height / 2;
}
