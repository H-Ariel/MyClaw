#pragma once

#include "MenuEngine.h"
#include "LevelHUD.h"


struct ClawLevelEngineFields
{
	ClawLevelEngineFields(int levelNumber);
	~ClawLevelEngineFields();

	shared_ptr<WapWorld> _wwd;
	D2D1_POINT_2F* _mainPlanePosition;
	LevelHUD* _hud;
	MenuBackgroundImage _helpImage;
	ColorF _saveBgColor;
	float _savePixelSize;
	const int _levelNumber;
};

class ClawLevelEngine : public BaseEngine
{
public:
	ClawLevelEngine(int levelNumber);
	ClawLevelEngine(shared_ptr<ClawLevelEngineFields> fields);

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnKeyDown(int key) override;

	shared_ptr<ClawLevelEngineFields> getFields() const { return _fields; }

private:
	shared_ptr<ClawLevelEngineFields> _fields; // save fields for easy access after ingame-menu
};
