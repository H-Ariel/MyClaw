#include "OpeningScreenEngine.h"
#include "MenuEngine/MenuEngine.h"
#include "Assets-Managers/AssetsManager.h"


constexpr auto OPENING_SOUND_WAV_PATH = "STATES/ATTRACT/SOUNDS/TITLE.WAV";

OpeningScreenEngine::OpeningScreenEngine()
	: ScreenEngine("STATES/ATTRACT/SCREENS/TITLE.PCX"), _wavId(AssetsManager::INVALID_AUDIOPLAYER_ID), _totalTime(0)
{
	_wavId = AssetsManager::playWavFile(OPENING_SOUND_WAV_PATH);
	_totalTime = (int)AssetsManager::getWavFileDuration(OPENING_SOUND_WAV_PATH);
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
	changeEngine(make_shared<MenuEngine>());
}
