#pragma once

#include "GameEngine/UIBaseImage.h"


// an image whose size is determined by window's size
class MenuBackgroundImage : public UIBaseImage
{
public:
	MenuBackgroundImage(const string& imagePath);
	void Logic(uint32_t elapsedTime) override;

private:
	const float _imgRatio; // `width / height`
};
