#pragma once

#include "BackgroundImage.h"


class ScreenRelative
{
protected:
	ScreenRelative(D2D1_POINT_2F posRatio, BackgroundImage* bgImg)
		: _posRatio(posRatio), _bgImg(bgImg), _sizeRatio({}) { }

	void updateSizeAndPosition(D2D1_SIZE_F& size, D2D1_POINT_2F& position) const
	{
		size.width = _bgImg->size.width / _sizeRatio.width;
		size.height = _bgImg->size.height / _sizeRatio.height;
		position.x = _bgImg->position.x + _posRatio.x * _bgImg->size.width;
		position.y = _bgImg->position.y + _posRatio.y * _bgImg->size.height;
	}


	D2D1_POINT_2F _posRatio;
	D2D1_SIZE_F _sizeRatio;
	BackgroundImage* _bgImg; // save it to get the size and position, so we can save item's image ratio
};
