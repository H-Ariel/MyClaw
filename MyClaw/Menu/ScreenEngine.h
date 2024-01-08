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

	static void setIngameMenu();
	static void setMainMenu();
	static void clearClawLevelEngineFields();

protected:
	virtual void backToMenu();

	static shared_ptr<ClawLevelEngineFields> _clawLevelEngineFields;

	MenuBackgroundImage* _bgImg;
};
