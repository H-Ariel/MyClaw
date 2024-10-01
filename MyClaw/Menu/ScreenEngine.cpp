#include "ScreenEngine.h"
#include "GameEngine/WindowManager.h"


#define DEFAULT_BG_IMAGE "STATES/MENU/SCREENS/MENU.PCX"


shared_ptr<ClawLevelEngineFields> ScreenEngine::_clawLevelEngineFields;


ScreenEngine::ScreenEngine(const string& bgPcxPath)
{
	_bgImg = DBG_NEW MenuBackgroundImage(bgPcxPath.empty() ? DEFAULT_BG_IMAGE : bgPcxPath);
	_elementsList.push_back(_bgImg);
	clearScreen = true;

	WindowManager::setWindowOffset({});
	WindowManager::setBackgroundColor(ColorF::Black);
	WindowManager::setWindowScale(1);
}
ScreenEngine::ScreenEngine(shared_ptr<ClawLevelEngineFields> fields, const string& bgPcxPath)
	: ScreenEngine(bgPcxPath)
{
	_clawLevelEngineFields = fields;
}

ScreenEngine::~ScreenEngine()
{
	for (UIBaseElement* i : _elementsList) delete i;
}
