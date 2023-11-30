#pragma once

#include "ScreenEngine.h"


class LevelLoadingEngine : public ScreenEngine
{
public:
	LevelLoadingEngine(int lvlNo);
	void Logic(uint32_t elapsedTime);

private:
	uint32_t _totalTime;
	const int _lvlNo;
	bool _isDrawn;
};
