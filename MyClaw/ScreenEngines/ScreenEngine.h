#pragma once

#include "GameEngine/BaseEngine.h"
#include "BackgroundImage.h"


class ScreenEngine : public BaseEngine
{
public:
	ScreenEngine(const string& bgPcxPath = "");
	~ScreenEngine();

protected:
	BackgroundImage* _bgImg;
};
