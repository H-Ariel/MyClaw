#include "UIColorfullyImage.h"


const ColorF TransparentPixel(0, 0, 0, 0);

UIColorfullyImage::UIColorfullyImage(const map<ColorF, ID2D1Bitmap*, function<bool(const ColorF&, const ColorF&)>>& images, D2D1_POINT_2F offset)
	: UIBaseImage(images.at(TransparentPixel), offset), _images(images)
{
}

void UIColorfullyImage::setCurrentColor(const ColorF* color)
{
	auto it = color ? _images.find(*color) : _images.end();
	if (it == _images.end())
		_bitmap = _images.at(TransparentPixel);
	else
		_bitmap = it->second;
}

shared_ptr<UIBaseImage> UIColorfullyImage::getCopy()
{
	return make_shared<UIColorfullyImage>(*this);
}
