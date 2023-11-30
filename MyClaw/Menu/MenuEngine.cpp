#include "MenuEngine.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../GUI/WindowManager.h"
#include "../ClawLevelEngine.h"
#include "../BasePlaneObject.h"
#include "HierarchicalMenu.h"
#include "HelpScreenEngine.h"
#include "CreditsEngine.h"
#include "LevelLoadingEngine.h"
#include "MenuItem.h"


stack<const HierarchicalMenu*> MenuEngine::_menusStack;
const HierarchicalMenu* MenuEngine::_currMenu = &HierarchicalMenu::MainMenu;


MenuEngine::MenuEngine() : MenuEngine(true, "") {}
MenuEngine::MenuEngine(const string& bgPcxPath) : MenuEngine(false, bgPcxPath) {}
MenuEngine::MenuEngine(bool allocChildren, const string& bgPcxPath) : MenuEngine({}, nullptr, allocChildren, bgPcxPath) {}
MenuEngine::MenuEngine(D2D1_POINT_2U mPos, shared_ptr<Animation> cursor, bool allocChildren, const string& bgPcxPath)
	: ScreenEngine(bgPcxPath)
{
	mousePosition = mPos;
	WindowManager::setTitle("Claw"); // TODO: change title according to the menu
	

	if (!allocChildren)
	{
		return;
	}

	if (cursor)
	{
		_cursor = cursor;
	}
	else if (_currMenu == &HierarchicalMenu::MainMenu || _currMenu == &HierarchicalMenu::InGameMenu)
	{
		AssetsManager::loadPidPalette("LEVEL1/PALETTES/MAIN.PAL");
		_cursor = AssetsManager::loadAnimation("STATES/MENU/ANIS/CURSOR.ANI");
	}

	function<void(MouseButtons)> onClick;

	for (const HierarchicalMenu& m : _currMenu->subMenus)
	{
		switch (m.cmd)
		{
		case HierarchicalMenu::Nop:
			onClick = nullptr;
			break;

		case HierarchicalMenu::NotImpleted:
			onClick = [](MouseButtons) { MessageBoxA(nullptr, "not impleted", "", 0); };
			break;

		case HierarchicalMenu::MenuIn:
			onClick = [&](MouseButtons) {
				_menusStack.push(_currMenu);
				_currMenu = &m;
				changeEngine<MenuEngine>(mousePosition, _cursor);
			};
			break;

		case HierarchicalMenu::MenuOut:
			onClick = [&](MouseButtons) {
				if (_menusStack.size() > 0)
				{
					_currMenu = _menusStack.top();
					_menusStack.pop();
					changeEngine<MenuEngine>(mousePosition, _cursor);
				}
			};
			break;

		case HierarchicalMenu::ExitApp:
			onClick = [&](MouseButtons) {
				_nextEngine = nullptr;
				StopEngine = true;
			};
			break;

		case HierarchicalMenu::Help:
			onClick = [&](MouseButtons) {
				_menusStack.push(_currMenu);
				_currMenu = &m;
				changeEngine<HelpScreenEngine>();
			};
			break;

		case HierarchicalMenu::Credits:
			onClick = [&](MouseButtons) {
				_menusStack.push(_currMenu);
				_currMenu = &m;
				changeEngine<CreditsEngine>();
			};
			break;

		case HierarchicalMenu::BackToGame:
			onClick = [&](MouseButtons) {
				_currMenu = &HierarchicalMenu::InGameMenu; // reset the menu
				while (_menusStack.size()) _menusStack.pop();
				changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
			};
			break;

		case HierarchicalMenu::EndLife:
			onClick = [&](MouseButtons) {
				BasePlaneObject::player->loseLife();
				_currMenu = &HierarchicalMenu::InGameMenu; // reset the menu
				while (_menusStack.size()) _menusStack.pop();
				changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
			};
			break;

		case HierarchicalMenu::EndGame:
			onClick = [&](MouseButtons) {
				AssetsManager::stopBackgroundMusic();
				_currMenu = &HierarchicalMenu::MainMenu; // reset the menu
				while (_menusStack.size()) _menusStack.pop();
				changeEngine<MenuEngine>();
			};
			break;

		default:
			if ((m.cmd & HierarchicalMenu::OpenLevel) == HierarchicalMenu::OpenLevel)
			{
				onClick = [&](MouseButtons) {
					_currMenu = &HierarchicalMenu::InGameMenu; // reset the menu
					while (_menusStack.size()) _menusStack.pop();
					_clawLevelEngineFields.reset(); // do not recycle the fields!
					changeEngine<LevelLoadingEngine>((m.cmd & 0xf0) >> 4);
				};
			}
			break;
		}

		_elementsList.push_back((UIBaseImage*)(DBG_NEW MenuItem(m.pcxPath, m.xRatio, m.yRatio, onClick, _bgImg, this)));
	}

	if (_cursor)
	{
		_elementsList.push_back(_cursor.get());
	}
}
MenuEngine::MenuEngine(shared_ptr<ClawLevelEngineFields> clawLevelEngineFields, bool allocChildren, const string& bgPcxPath)
	: MenuEngine(allocChildren, bgPcxPath)
{
	_clawLevelEngineFields = clawLevelEngineFields;
}
MenuEngine::~MenuEngine()
{
	if (_cursor) _elementsList.pop_back(); // remove the cursor
}

void MenuEngine::Logic(uint32_t elapsedTime) {
	if (_cursor) _cursor->position = { mousePosition.x + 16.f, mousePosition.y + 17.f };
	ScreenEngine::Logic(elapsedTime);
}
