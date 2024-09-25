#pragma once

#include "GameEngine/UIBaseButton.h"
#include "MenuEngine.h"


class MenuItem : public UIBaseButton
{
public:
	MenuItem(const string& imagePath, float xRatio, float yRatio,
		MenuBackgroundImage* bgImg, ScreenEngine* parent);

	MenuItem(const string& imagePath, const string& markedImagePath, float xRatio, float yRatio,
		function<void(MenuItem*)> itemOnClick, MenuBackgroundImage* bgImg, ScreenEngine* parent);

	MenuItem(const string& imagePath, const string& markedImagePath,
		const string& toggleStateImage, const string& markedToggleStateImage,
		float xRatio, float yRatio,
		function<void(MenuItem*)> itemOnClick, MenuBackgroundImage* bgImg, ScreenEngine* parent, bool initialState);

	void Logic(uint32_t) override;
	void Draw() override;

	virtual bool isActive() const;

	void setStates(bool s) { state = s; }
	void switchState() { state = !state; }

	bool marked;

protected:
	function<void(MenuItem*)> _itemOnClick; // send `this` as parameter
	D2D1_POINT_2F _posRatio;
	D2D1_SIZE_F _sizeRatio;
	MenuBackgroundImage* _bgImg; // save it to get the size and position, so we can save item's image ratio
	shared_ptr<UIBaseImage> _image, _markedImage;
	shared_ptr<UIBaseImage> _toggleStateImage, _markedToggleStateImage;
	bool state; // 0 for _image, 1 for _toggleStateImage
};


class MenuSlider : public MenuItem
{
public:
	MenuSlider(const string& imagePath, const string& markedImagePath,
		const string& toggleStateImage, const string& markedToggleStateImage,
		float xRatio, float yRatio,
		function<void(MenuSlider*, int)> onMove,
		MenuBackgroundImage* bgImg, ScreenEngine* parent, int initialValue);

	void Draw() override;
	bool isActive() const override;

	void moveSlider(int step);
	int getValue() const { return _value; }


private:
	function<void(MenuSlider*, int)> _onMove; // send `this` and the new value as parameters
	D2D1_POINT_2F _thumbPosRatio;
	D2D1_SIZE_F _thumbSizeRatio;
	shared_ptr<UIBaseImage> thumbImage, markedThumbImage;
	int _value;
};
