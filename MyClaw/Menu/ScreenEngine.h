#pragma once

#include "GameEngine/BaseEngine.h"
#include "MenuBackgroundImage.h"


class ScreenEngine : public BaseEngine
{
public:
	ScreenEngine(const string& bgPcxPath = "");
	~ScreenEngine();

protected:
	MenuBackgroundImage* _bgImg;
};
