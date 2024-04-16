#include "MenuBackgroundImage.h"
#include "../Assets-Managers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


MenuBackgroundImage::MenuBackgroundImage(const string& pcxPath)
	: UIBaseImage(*AssetsManager::loadImage(pcxPath)), _imgRatio(size.width / size.height) {}
void MenuBackgroundImage::Logic(uint32_t)
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
