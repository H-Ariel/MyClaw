#include "MenuItem.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../GUI/WindowManager.h"


#define PLAY_SELECT_SOUND AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV")


MenuItem::MenuItem(const string& pcxPath, float xRatio, float yRatio,
	function<void(MouseButtons)> onClick, MenuBackgroundImage* bgImg, ScreenEngine* parent)
	: UIBaseButton(onClick, parent), UIBaseImage(*AssetsManager::loadImage(pcxPath)),
	_posRatio({ xRatio, yRatio }), _bgImg(bgImg)
{
	_sizeRatio = {
		WindowManager::DEFAULT_WINDOW_SIZE.width / UIBaseImage::size.width,
		WindowManager::DEFAULT_WINDOW_SIZE.height / UIBaseImage::size.height
	};

	_onClick2 = onClick; // save the original onClick function
	this->onClick = [&](MouseButtons btn) { // override onClick function so it will play the select sound
		PLAY_SELECT_SOUND;
		_onClick2(btn);
	};
}

void MenuItem::Logic(uint32_t)
{
	UIBaseButton::size.width = _bgImg->size.width / _sizeRatio.width;
	UIBaseButton::size.height = _bgImg->size.height / _sizeRatio.height;
	UIBaseButton::position.x = _bgImg->position.x + _posRatio.x * _bgImg->size.width;
	UIBaseButton::position.y = _bgImg->position.y + _posRatio.y * _bgImg->size.height;
	UIBaseImage::position = UIBaseButton::position;
	UIBaseImage::size = UIBaseButton::size;
	UIBaseButton::Logic(0); // just to update the size and position
}

void MenuItem::mulImageSizeRatio(float n)
{
	_sizeRatio.width *= n;
	_sizeRatio.height *= n;
}
