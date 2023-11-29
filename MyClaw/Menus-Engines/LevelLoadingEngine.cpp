#include "LevelLoadingEngine.h"
#include "../ClawLevelEngine.h"


LevelLoadingEngine::LevelLoadingEngine(int lvlNo)
	: MenuEngine("LEVEL" + to_string(lvlNo) + "/SCREENS/LOADING.PCX"),
	_totalTime(0), _lvlNo(lvlNo), _isDrawn(false) {}
void LevelLoadingEngine::Logic(uint32_t elapsedTime)
{
	_bgImg->Logic(elapsedTime);
	_totalTime += elapsedTime;
	if (_isDrawn && _totalTime > 100) {
		changeEngine<ClawLevelEngine>(_lvlNo);
		_currMenu = &HierarchicalMenu::InGameMenu;
	}
	_isDrawn = true; // after once it is sure to be drawn.
}
