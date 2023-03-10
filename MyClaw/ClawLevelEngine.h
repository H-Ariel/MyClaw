#pragma once

#include "MenuEngine.h"
#include "LevelMap.h"
#include "LevelHUD.h"


class ClawLevelEngine : public BaseEngine
{
public:
	ClawLevelEngine(int8_t levelNumber);
	~ClawLevelEngine() override;

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnKeyDown(int key) override;


private:
	enum class State : int8_t { Play, Help };

	LevelMap* _levelMap;
	LevelHUD* _hud;
	Player* _player;
	MenuBackgroundImage* _helpImage;
	ColorF saveBgColor;
	State _state;
	const int8_t _levelNumber;
};
