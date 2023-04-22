#pragma once

#include "MenuEngine.h"
#include "LevelPlane.h"
#include "LevelHUD.h"


class ClawLevelEngine : public BaseEngine
{
public:
	ClawLevelEngine(int levelNumber);
	~ClawLevelEngine();

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnKeyDown(int key) override;


private:
	enum class State : int8_t { Play, Help };

	shared_ptr<WapWorld> _wwd;
	LevelPlane* _mainPlane;
	LevelHUD* _hud;
	Player* _player;
	MenuBackgroundImage _helpImage;
	ColorF _saveBgColor;
	State _state;
	const int _levelNumber;
};
