#pragma once

#include "GameEngine/BaseEngine.h"
#include "MenuBackgroundImage.h"


struct ClawLevelEngineFields;

class ScreenEngine : public BaseEngine
{
public:
	ScreenEngine(const string& bgPcxPath = "");
	ScreenEngine(shared_ptr<ClawLevelEngineFields> fields, const string& bgPcxPath);
	~ScreenEngine();

protected:
	static shared_ptr<ClawLevelEngineFields> _clawLevelEngineFields;

	MenuBackgroundImage* _bgImg;
};
