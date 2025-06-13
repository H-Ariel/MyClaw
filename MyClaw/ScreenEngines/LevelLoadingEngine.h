#pragma once

#include "ScreenEngine.h"


class LevelLoadingEngine : public ScreenEngine
{
public:
	LevelLoadingEngine(int level, int checkpoint = -1);
	void Logic(uint32_t elapsedTime);

private:
	const int _level, _checkpoint;
	int _totalTime;
};
