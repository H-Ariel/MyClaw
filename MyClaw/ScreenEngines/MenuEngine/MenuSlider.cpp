#include "MenuSlider.h"
#include "AssetsManagers/AssetsManager.h"
#include "GameEngine/WindowManager.h"


MenuSlider::MenuSlider(const string& imagePath, const string& markedImagePath,
	const string& toggleStateImage, const string& markedToggleStateImage, float xRatio, float yRatio,
	function<void(MenuSlider*, int)> onMove, BackgroundImage* bgImg, MenuEngine* parent, int initialValue)
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
