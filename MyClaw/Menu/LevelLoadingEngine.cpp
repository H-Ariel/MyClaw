#include "LevelLoadingEngine.h"
#include "../ClawLevelEngine.h"


LevelLoadingEngine::LevelLoadingEngine(int level, int checkpoint)
	: ScreenEngine("LEVEL" + to_string(level) + "/SCREENS/LOADING.PCX"),
	_level(level), _checkpoint(checkpoint), _totalTime(0), _isDrawn(false)
{
}

void LevelLoadingEngine::Logic(uint32_t elapsedTime)
{
	_bgImg->Logic(elapsedTime);
	_totalTime += elapsedTime;
	if (_isDrawn && _totalTime > 100) {
		changeEngine<ClawLevelEngine>(_level, _checkpoint);
		setIngameMenu();
		clearClawLevelEngineFields();
	}
	_isDrawn = true; // after once it is sure to be drawn.
}
