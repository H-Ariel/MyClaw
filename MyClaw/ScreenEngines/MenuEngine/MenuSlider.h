#pragma once

#include "MenuItem.h"


class MenuSlider : public MenuItem
{
public:
	MenuSlider(const string& imagePath, const string& markedImagePath,
		const string& toggleStateImage, const string& markedToggleStateImage,
		float xRatio, float yRatio,
		function<void(MenuSlider*, int)> onMove,
		BackgroundImage* bgImg, MenuEngine* parent, int initialValue);

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
