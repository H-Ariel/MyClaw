#include "LevelLoadingEngine.h"
#include "MenuEngine.h"
#include "../ClawLevelEngine.h"


LevelLoadingEngine::LevelLoadingEngine(int level, int checkpoint)
	: ScreenEngine("LEVEL" + to_string(level) + "/SCREENS/LOADING.PCX"),
	_level(level), _checkpoint(checkpoint), _totalTime(0)
{
	LogFile::log(LogFile::Info, "load level %d", level);
}

void LevelLoadingEngine::Logic(uint32_t elapsedTime)
{
	_bgImg->Logic(elapsedTime);
	_totalTime += elapsedTime;
	if (_totalTime > 100) { // wait 100ms
		changeEngine<ClawLevelEngine>(_level, _checkpoint);
		_clawLevelEngineFields.reset();
	}
}
