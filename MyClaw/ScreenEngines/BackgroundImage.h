#pragma once

#include "GameEngine/UIBaseImage.h"


// an image whose size is determined by window's size
class BackgroundImage : public UIBaseImage
{
public:
	BackgroundImage(const string& imagePath);
	void Logic(uint32_t elapsedTime) override;

private:
	const float _imgRatio; // `width / height`
};
