#include "MenuEngine.h"
#include "../GUI/WindowManager.h"


#define DEFAULT_BG_IMAGE "STATES/MENU/SCREENS/MENU.PCX"


shared_ptr<ClawLevelEngineFields> ScreenEngine::_clawLevelEngineFields;


ScreenEngine::ScreenEngine(const string& bgPcxPath)
{
	_bgImg = DBG_NEW MenuBackgroundImage(bgPcxPath.empty() ? DEFAULT_BG_IMAGE : bgPcxPath);
	_elementsList.push_back(_bgImg);

	WindowManager::setWindowOffset(nullptr);
	WindowManager::setBackgroundColor(ColorF::Black);
	WindowManager::PixelSize = 1;
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
void ScreenEngine::backToMenu()
{
	MenuEngine::_currMenu = MenuEngine::_menusStack.top();
	MenuEngine::_menusStack.pop();
	changeEngine<MenuEngine>();
}

void ScreenEngine::setIngameMenu()
{
	MenuEngine::_currMenu = &HierarchicalMenu::InGameMenu;
	while (MenuEngine::_menusStack.size()) MenuEngine::_menusStack.pop();
}
void ScreenEngine::setMainMenu()
{
	MenuEngine::_currMenu = &HierarchicalMenu::MainMenu;
	while (MenuEngine::_menusStack.size()) MenuEngine::_menusStack.pop();
}
void ScreenEngine::clearClawLevelEngineFields()
{
	_clawLevelEngineFields.reset();
}
