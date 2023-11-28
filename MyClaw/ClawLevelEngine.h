#pragma once

#include "MenuEngine.h"
#include "LevelPlane.h"
#include "LevelHUD.h"


class ClawLevelEngine : public BaseEngine
{
public:
	ClawLevelEngine(int levelNumber);
	~ClawLevelEngine() override;

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnKeyDown(int key) override;

	void setSharedPtr(shared_ptr<ClawLevelEngine> spThis);

private:
	enum class State : int8_t { Play, Pause };

	shared_ptr<WapWorld> _wwd;
	shared_ptr<ClawLevelEngine> _spThis; // save `this` as `shared_ptr`. used when switchto menu.
	D2D1_POINT_2F* _mainPlanePosition;
	LevelHUD* _hud;
	MenuBackgroundImage _helpImage;
	ColorF _saveBgColor;
	float _savePixelSize;
	const int _levelNumber;
	State _state;
};
