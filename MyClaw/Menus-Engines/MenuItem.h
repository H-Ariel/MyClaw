#pragma once

#include "../GUI/UIBaseButton.h"
#include "MenuEngine.h"


class MenuItem : virtual UIBaseButton, virtual UIBaseImage
{
	using UIBaseImage::Draw;

public:
	MenuItem(string pcxPath, float xRatio, float yRatio,
		function<void(MouseButtons)> onClick, MenuBackgroundImage* bgImg, MenuEngine* parent);

	void Logic(uint32_t) override;

	void mulImageSizeRatio(float n); // multiply the size ratio by n

private:
	D2D1_POINT_2F _posRatio;
	D2D1_SIZE_F _sizeRatio;
	MenuBackgroundImage* _bgImg;
};
