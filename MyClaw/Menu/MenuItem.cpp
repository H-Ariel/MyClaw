#include "MenuItem.h"
#include "../Assets-Managers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


MenuItem::MenuItem(const string& pcxPath, const string& markedPcxPath, float xRatio, float yRatio,
	function<void(MouseButtons)> onClick, MenuBackgroundImage* bgImg, ScreenEngine* parent)
	: UIBaseButton(onClick, parent), _posRatio({ xRatio, yRatio }), _bgImg(bgImg), marked(false)
{
	_image = AssetsManager::loadImage(pcxPath)->getCopy(); // do not modify the original image, it makes the menu smaller each time it is opened
	if (!markedPcxPath.empty())
		_markedImage = AssetsManager::loadImage(markedPcxPath)->getCopy();

	_sizeRatio.width = WindowManager::DEFAULT_WINDOW_SIZE.width / _image->size.width;
	_sizeRatio.height = WindowManager::DEFAULT_WINDOW_SIZE.height / _image->size.height;

	if (onClick)
	{
		_onClick2 = onClick; // save the original onClick function
		this->onClick = [&](MouseButtons btn) { // override onClick function so it will play the select sound
			AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
			_onClick2(btn);
		};
	}

	Logic(0); // to update the size and position
}

void MenuItem::Logic(uint32_t)
{
	size.width = _bgImg->size.width / _sizeRatio.width;
	size.height = _bgImg->size.height / _sizeRatio.height;
	position.x = _bgImg->position.x + _posRatio.x * _bgImg->size.width;
	position.y = _bgImg->position.y + _posRatio.y * _bgImg->size.height;
	UIBaseButton::Logic(0); // just to update the size and position
}
void MenuItem::Draw()
{
	shared_ptr<UIBaseImage> imgToDraw = (marked && _markedImage) ? _markedImage : _image;
	imgToDraw->size = size;
	imgToDraw->position = position;
	imgToDraw->Draw();
}

void MenuItem::mulImageSizeRatio(float n)
{
	_sizeRatio.width *= n;
	_sizeRatio.height *= n;
}
