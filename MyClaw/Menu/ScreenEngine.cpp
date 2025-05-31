#include "ScreenEngine.h"
#include "GameEngine/WindowManager.h"


constexpr auto DEFAULT_BG_IMAGE = "STATES/MENU/SCREENS/MENU.PCX";


ScreenEngine::ScreenEngine(const string& bgPcxPath)
{
	_bgImg = DBG_NEW MenuBackgroundImage(bgPcxPath.empty() ? DEFAULT_BG_IMAGE : bgPcxPath);
	_elementsList.push_back(_bgImg);
	clearScreen = true;

	WindowManager::setWindowOffset({});
	WindowManager::setBackgroundColor(ColorF::Black);
	WindowManager::setWindowScale(1);
}

ScreenEngine::~ScreenEngine()
{
	for (UIBaseElement* i : _elementsList) delete i;
}
