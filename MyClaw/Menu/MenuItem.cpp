#include "MenuItem.h"
#include "../Assets-Managers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


MenuItem::MenuItem(const string& pcxPath, float xRatio, float yRatio,
	MenuBackgroundImage* bgImg, ScreenEngine* parent)
	: MenuItem(pcxPath, "", xRatio, yRatio, nullptr, bgImg, parent) {}

MenuItem::MenuItem(const string& pcxPath, const string& markedPcxPath, float xRatio, float yRatio,
	function<void(MenuItem*)> onClick, MenuBackgroundImage* bgImg, ScreenEngine* parent)
	: MenuItem(pcxPath, markedPcxPath, "", "", xRatio, yRatio, onClick, bgImg, parent, 0) {}

MenuItem::MenuItem(const string& pcxPath, const string& markedPcxPath,
	const string& pcxPath2, const string& markedPcxPath2, float xRatio, float yRatio,
	function<void(MenuItem*)> itemOnClick, MenuBackgroundImage* bgImg, ScreenEngine* parent, int initialState)
	: UIBaseButton(nullptr, parent), _posRatio({ xRatio, yRatio }), _bgImg(bgImg), marked(false), state(initialState)
{
	_image = AssetsManager::loadImage(pcxPath)->getCopy(); // do not modify the original image, it makes the menu smaller each time it is opened
	if (!markedPcxPath.empty())
		_markedImage = AssetsManager::loadImage(markedPcxPath)->getCopy();
	if (!pcxPath2.empty())
		_image2 = AssetsManager::loadImage(pcxPath2)->getCopy();
	if (!markedPcxPath2.empty())
		_markedImage2 = AssetsManager::loadImage(markedPcxPath2)->getCopy();

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
	shared_ptr<UIBaseImage> imgToDraw = (marked && _markedImage) ? (state == 0 ? _markedImage : _markedImage2) : (state == 0 ? _image : _image2);
	imgToDraw->size = size;
	imgToDraw->position = position;
	imgToDraw->Draw();
}

void MenuItem::mulImageSizeRatio(float n)
{
	_sizeRatio.width *= n;
	_sizeRatio.height *= n;
}
