#pragma once

#include "GameEngine/UIBaseButton.h"
#include "MenuEngine.h"


class MenuItem : public UIBaseButton
{
public:
	MenuItem(const string& pcxPath, const string& markedPcxPath, float xRatio, float yRatio,
		function<void(MouseButtons)> onClick, MenuBackgroundImage* bgImg, ScreenEngine* parent);

	void Logic(uint32_t) override;
	void Draw() override;

	void mulImageSizeRatio(float n); // multiply the size ratio by n

	bool isActive() const { return (bool)onClick; }

	bool marked;

private:
	function<void(MouseButtons)> _onClick2; // save the original onClick function so we can wrap it with the select sound
	D2D1_POINT_2F _posRatio;
	D2D1_SIZE_F _sizeRatio;
	MenuBackgroundImage* _bgImg; // save it to get the size and position, so we can save item's image ratio
	shared_ptr<UIBaseImage> _image, _markedImage;
};
