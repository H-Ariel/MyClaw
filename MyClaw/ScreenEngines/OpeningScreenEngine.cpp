#include "OpeningScreenEngine.h"
#include "MenuEngine/MenuEngine.h"
#include "AssetsManagers/AssetsManager.h"


constexpr auto OPENING_SOUND_WAV_PATH = "STATES/ATTRACT/SOUNDS/TITLE.WAV";

OpeningScreenEngine::OpeningScreenEngine()
	: ScreenEngine("STATES/ATTRACT/SCREENS/TITLE.PCX"), _wavId(AssetsManager::INVALID_AUDIOPLAYER_ID)
{
	_wavId = AssetsManager::playWavFile(OPENING_SOUND_WAV_PATH);

	timer = DBG_NEW Timer((int)AssetsManager::getWavFileDuration(OPENING_SOUND_WAV_PATH),
		bind(&OpeningScreenEngine::continueToMenu, this));
	_timers.push_back(timer);
}
OpeningScreenEngine::~OpeningScreenEngine() { delete timer; }
void OpeningScreenEngine::OnKeyUp(int key) { continueToMenu(); }
void OpeningScreenEngine::OnMouseButtonUp(MouseButtons btn) { continueToMenu(); }
void OpeningScreenEngine::continueToMenu()
{
	if (!timer->isFinished())
		AssetsManager::stopWavFile(_wavId);
	changeEngine(make_shared<MenuEngine>());
}
