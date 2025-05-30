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


MenuSlider::MenuSlider(const string& imagePath, const string& markedImagePath,
	const string& toggleStateImage, const string& markedToggleStateImage, float xRatio, float yRatio,
	function<void(MenuSlider*, int)> onMove, MenuBackgroundImage* bgImg, ScreenEngine* parent, int initialValue)
	: MenuItem(imagePath, markedImagePath, toggleStateImage, markedToggleStateImage, xRatio, yRatio,
		[&](MenuItem*) { moveSlider(_value == 0 ? 10 : -10); }, bgImg, parent, 0),
	_onMove(onMove), _value(initialValue)
{
	thumbImage = AssetsManager::loadImage(OPTIONS_ROOT "AUDIO/041.PCX")->getCopy();
	markedThumbImage = AssetsManager::loadImage(OPTIONS_ROOT "AUDIO/042.PCX")->getCopy();

	_thumbSizeRatio.width = WindowManager::DEFAULT_WINDOW_SIZE.width / thumbImage->size.width;
	_thumbSizeRatio.height = WindowManager::DEFAULT_WINDOW_SIZE.height / thumbImage->size.height;
	_thumbPosRatio.y = yRatio - 0.005f;
	moveSlider(0); // to update the thumb position
}
void MenuSlider::Draw()
{
	MenuItem::Draw();

	shared_ptr<UIBaseImage> imgToDraw = marked ? markedThumbImage : thumbImage;
	imgToDraw->size.width = _bgImg->size.width / _thumbSizeRatio.width;
	imgToDraw->size.height = _bgImg->size.height / _thumbSizeRatio.height;
	imgToDraw->position.x = _bgImg->position.x + _thumbPosRatio.x * _bgImg->size.width;
	imgToDraw->position.y = _bgImg->position.y + _thumbPosRatio.y * _bgImg->size.height;
	imgToDraw->Draw();
}
bool MenuSlider::isActive() const { return true; }
void MenuSlider::moveSlider(int step)
{
	_value += step;
	if (_value > 9) _value = 9;
	else if (_value < 0) _value = 0;
	_thumbPosRatio.x = _posRatio.x + 0.09f + 0.0335f * _value; // TODO: find perfect factors (offset=0.09f, rate=0.0335f)

	_onMove(this, _value);
}
