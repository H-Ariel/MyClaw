#include "OpeningScreenEngine.h"
#include "MenuEngine.h"
#include "Assets-Managers/AssetsManager.h"


OpeningScreenEngine::OpeningScreenEngine()
	: ScreenEngine("STATES/ATTRACT/SCREENS/TITLE.PCX")
{
	_wavId = AssetsManager::playWavFile("STATES/ATTRACT/SOUNDS/TITLE.WAV");
	_totalTime = (int)AssetsManager::getWavFileDuration(_wavId);
}
void OpeningScreenEngine::Logic(uint32_t elapsedTime)
{
	BaseEngine::Logic(elapsedTime);
	_totalTime -= elapsedTime;
	if (_totalTime <= 0)
		continueToMenu();
}
void OpeningScreenEngine::OnKeyUp(int key) { continueToMenu(); }
void OpeningScreenEngine::OnMouseButtonUp(MouseButtons btn) { continueToMenu(); }
void OpeningScreenEngine::continueToMenu()
{
	if (_totalTime > 0)
		AssetsManager::stopWavFile(_wavId);
	changeEngine<MenuEngine>();
}
