#pragma once

#include "UIBaseImage.h"


class UIColorfulImage : public UIBaseImage
{
public:
	UIColorfulImage(const map<ColorF, ID2D1Bitmap*, function<bool(const ColorF&, const ColorF&)>>& images, D2D1_POINT_2F offset = {});

	void setCurrentColor(const ColorF* color);

	shared_ptr<UIBaseImage> getCopy() override;

private:
	const map<ColorF, ID2D1Bitmap*, function<bool(const ColorF&, const ColorF&)>> _images;
};