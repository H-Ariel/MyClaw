#include "LevelLoadingEngine.h"
#include "MenuEngine.h"
#include "LevelEngine/ClawLevelEngine.h"


LevelLoadingEngine::LevelLoadingEngine(int level, int checkpoint)
	: ScreenEngine("LEVEL" + to_string(level) + "/SCREENS/LOADING.PCX"),
	_level(level), _checkpoint(checkpoint), _totalTime(0) { }

void LevelLoadingEngine::Logic(uint32_t elapsedTime)
{
	_bgImg->Logic(elapsedTime);
	_totalTime += elapsedTime;
	if (_totalTime > 100) { // wait 100ms
		changeEngine(DBG_NEW ClawLevelEngine(_level, _checkpoint));
		_clawLevelEngineFields.reset();
	}
}
