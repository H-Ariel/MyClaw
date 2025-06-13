#pragma once

#include "ScreenRelative.h"


class AdaptiveImage : public UIBaseImage, protected ScreenRelative
{
public:
	AdaptiveImage(const string& imagePath, D2D1_POINT_2F posRatio, BackgroundImage* bgImg);

	void Logic(uint32_t) override;
};
