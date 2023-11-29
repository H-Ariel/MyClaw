#pragma once

#include "MenuEngine.h"


class LevelLoadingEngine : public MenuEngine
{
public:
	LevelLoadingEngine(int lvlNo);
	void Logic(uint32_t elapsedTime);

private:
	uint32_t _totalTime;
	const int _lvlNo;
	bool _isDrawn;
};
