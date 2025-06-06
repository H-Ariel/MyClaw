#include "MenuItem.h"
#include "Assets-Managers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


MenuItem::MenuItem(const string& imagePath, float xRatio, float yRatio,
	MenuBackgroundImage* bgImg, ScreenEngine* parent)
	: MenuItem(imagePath, "", xRatio, yRatio, nullptr, bgImg, parent) {}

MenuItem::MenuItem(const string& imagePath, const string& markedImagePath, float xRatio, float yRatio,
	function<void(MenuItem*)> itemOnClick, MenuBackgroundImage* bgImg, ScreenEngine* parent)
	: MenuItem(imagePath, markedImagePath, "", "", xRatio, yRatio, itemOnClick, bgImg, parent, 0) {}

MenuItem::MenuItem(const string& imagePath, const string& markedImagePath,
	const string& toggleStateImage, const string& markedToggleStateImage, float xRatio, float yRatio,
	function<void(MenuItem*)> itemOnClick, MenuBackgroundImage* bgImg, ScreenEngine* parent, bool initialState)
	: UIBaseButton(nullptr, parent), _posRatio({ xRatio, yRatio }), _bgImg(bgImg), marked(false), state(initialState)
{
	_image = AssetsManager::loadImage(imagePath)->getCopy(); // do not modify the original image, it makes the menu smaller each time it is opened
	if (!markedImagePath.empty())
		_markedImage = AssetsManager::loadImage(markedImagePath)->getCopy();
	if (!toggleStateImage.empty())
		_toggleStateImage = AssetsManager::loadImage(toggleStateImage)->getCopy();
	if (!markedToggleStateImage.empty())
		_markedToggleStateImage = AssetsManager::loadImage(markedToggleStateImage)->getCopy();

	_sizeRatio.width = WindowManager::DEFAULT_WINDOW_SIZE.width / _image->size.width;
	_sizeRatio.height = WindowManager::DEFAULT_WINDOW_SIZE.height / _image->size.height;

	if (itemOnClick)
	{
		_itemOnClick = itemOnClick; // save the original onClick function
		onClick = [&](MouseButtons) { // override onClick function so it will play the select sound
			AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
			_itemOnClick(this);
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
	shared_ptr<UIBaseImage> imgToDraw = (marked && _markedImage) ? (state ? _markedToggleStateImage : _markedImage) : (state ? _toggleStateImage : _image);
	imgToDraw->size = size;
	imgToDraw->position = position;
	imgToDraw->Draw();
}
bool MenuItem::isActive() const { return (bool)onClick; }
