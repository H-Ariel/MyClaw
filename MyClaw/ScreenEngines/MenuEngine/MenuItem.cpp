#include "MenuItem.h"
#include "AssetsManagers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


MenuItem::MenuItem(const string& imagePath, const string& markedImagePath,
	const string& toggleStateImage, const string& markedToggleStateImage, float xRatio, float yRatio,
	function<void(MenuItem*)> itemOnClick, BackgroundImage* bgImg, MenuEngine* parent, bool initialState)
	: UIBaseButton(nullptr, parent), ScreenRelative({ xRatio, yRatio },bgImg), marked(false),
	state(initialState ? ImageState::Toggled : ImageState::Normal)
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
	updateSizeAndPosition(this->size, this->position);
	UIBaseButton::Logic(0); // just to update the size and position
}
void MenuItem::Draw()
{
	shared_ptr<UIBaseImage> imgToDraw = (marked && _markedImage)
		? (state == ImageState::Toggled ? _markedToggleStateImage : _markedImage)
		: (state == ImageState::Toggled ? _toggleStateImage : _image);
	imgToDraw->size = size;
	imgToDraw->position = position;
	imgToDraw->Draw();
}
bool MenuItem::isActive() const { return (bool)onClick; }
