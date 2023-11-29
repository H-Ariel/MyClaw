#pragma once

#include "../GUI/UIBaseImage.h"


// an image whose size is determined by window's size
class MenuBackgroundImage : public UIBaseImage
{
public:
	MenuBackgroundImage(string pcxPath);
	void Logic(uint32_t elapsedTime) override;

private:
	const float _imgRatio; // `width / height`
};
