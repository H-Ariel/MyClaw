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


#define CLEAR_MENUS_STACK for (; _menusStack.size(); _menusStack.pop())


stack<const HierarchicalMenu*> MenuEngine::_menusStack;
const HierarchicalMenu* MenuEngine::_currMenu = &HierarchicalMenu::MainMenu;


MenuEngine::MenuEngine(const string& bgPcxPath) : MenuEngine({}, nullptr, bgPcxPath) {}
MenuEngine::MenuEngine(D2D1_POINT_2U mPos, shared_ptr<Animation> cursor, const string& bgPcxPath)
	: ScreenEngine(bgPcxPath)
{
	mousePosition = mPos;

	if (cursor)
	{
		_cursor = cursor;
	}
	else if (_currMenu == &HierarchicalMenu::MainMenu || _currMenu == &HierarchicalMenu::InGameMenu)
	{
		AssetsManager::loadPidPalette("LEVEL1/PALETTES/MAIN.PAL");
		_cursor = AssetsManager::loadAnimation("STATES/MENU/ANIS/CURSOR.ANI");
	}

	if (_currMenu == &HierarchicalMenu::MainMenu)
		WindowManager::setTitle("Claw");

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
			onClick = [&](MouseButtons) { menuIn(&m); };
			break;

		case HierarchicalMenu::MenuOut:
			onClick = [&](MouseButtons) { menuOut(); };
			break;

		case HierarchicalMenu::SelectLevel:
			onClick = [&](MouseButtons) {
				// change the title image:
				string dir = "";
				if (endsWith(m.pcxPath, "NEWGAME.PCX")) dir = "NEW";
				else if (endsWith(m.pcxPath, "LOADGAME.PCX")) dir = "LOAD";
				//else throw Exception("invalid pcx path: " + m.pcxPath);
				HierarchicalMenu::SelectLevelMenu.subMenus[0].pcxPath = SINGLEPLAYER_ROOT + dir + "/001_TITLE.PCX";
				menuIn(&HierarchicalMenu::SelectLevelMenu);
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
			onClick = [&](MouseButtons) { backToGame(); };
			break;

		case HierarchicalMenu::EndLife:
			onClick = [&](MouseButtons) {
				BasePlaneObject::player->loseLife(); // TODO: fall death
				CLEAR_MENUS_STACK;
				_currMenu = &HierarchicalMenu::InGameMenu;
				changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
			};
			break;

		case HierarchicalMenu::EndGame:
			onClick = [&](MouseButtons) {
				AssetsManager::stopBackgroundMusic();
				CLEAR_MENUS_STACK;
				_currMenu = &HierarchicalMenu::MainMenu;
				_clawLevelEngineFields.reset();
				changeEngine<MenuEngine>();
				BasePlaneObject::player = nullptr; // do not recycle the player in new game
			};
			break;

		default:
			// here we play with bits, string, and paths to get the level number and checkpoint number:

			if ((m.cmd & HierarchicalMenu::LoadCheckpoint) == HierarchicalMenu::LoadCheckpoint)
			{
				onClick = [&](MouseButtons) {
					int level = stoi(HierarchicalMenu::SelectCheckpoint.subMenus[0]
						.pcxPath.substr(strlen(LOAD_CHECKPOINT_ROOT), 3)) - 10;
					int checkpoint = (m.cmd & 0xf0) >> 4;
					char text[64] = {};
					sprintf(text, "load level %d checkpoint %d", level, checkpoint);
					MessageBoxA(nullptr, text, "not impleted", 0);

					// TODO: LevelLoadingEngine and checkpoint loading
				};
			}
			else if ((m.cmd & HierarchicalMenu::OpenLevel) == HierarchicalMenu::OpenLevel)
			{
				onClick = [&](MouseButtons) {
					int level = (m.cmd & 0xf0) >> 4;

					if (contains(_currMenu->subMenus[0].pcxPath, "NEW")) // new game
					{
						CLEAR_MENUS_STACK;
						_currMenu = &HierarchicalMenu::InGameMenu; // TODO: in LevelLoadingEngine ?
						_clawLevelEngineFields.reset(); // do not recycle the fields! // TODO: in LevelLoadingEngine ?
						changeEngine<LevelLoadingEngine>(level);
					}
					else if (contains(_currMenu->subMenus[0].pcxPath, "LOAD")) // load checkpoint
					{
						// set title:
						char num[64] = {}; sprintf(num, "%03d", level + 10);
						HierarchicalMenu::SelectCheckpoint.subMenus[0].pcxPath = LOAD_CHECKPOINT_ROOT + string(num) + "_TITLE.PCX";
						// TODO: check which checkpoints are available and change the menu-buttons accordingly
						menuIn(&HierarchicalMenu::SelectCheckpoint);
					}
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
MenuEngine::MenuEngine(shared_ptr<ClawLevelEngineFields> fields, const string& bgPcxPath)
	: MenuEngine(bgPcxPath)
{
	_clawLevelEngineFields = fields; // I don't have another way...
}
MenuEngine::~MenuEngine()
{
	if (_cursor) _elementsList.pop_back(); // remove the cursor
}

void MenuEngine::Logic(uint32_t elapsedTime) {
	if (_cursor) _cursor->position = { mousePosition.x + 16.f, mousePosition.y + 17.f };
	ScreenEngine::Logic(elapsedTime);
}
void MenuEngine::OnKeyUp(int key)
{
	if (key == VK_ESCAPE)
	{
		if (_currMenu == &HierarchicalMenu::MainMenu)
		{
			menuIn(&HierarchicalMenu::MainMenu.subMenus[7]); // quit-menu
		}
		else if (_currMenu == &HierarchicalMenu::MainMenu.subMenus[7])
		{
			menuOut(); // back to main-menu
		}
		else if (_currMenu == &HierarchicalMenu::InGameMenu)
		{
			backToGame();
		}
	}
}

void MenuEngine::menuIn(const HierarchicalMenu* newMenu)
{
	_menusStack.push(_currMenu);
	_currMenu = newMenu;
	changeEngine<MenuEngine>(mousePosition, _cursor);
}
void MenuEngine::menuOut()
{
	if (_menusStack.size() > 0)
	{
		_currMenu = _menusStack.top();
		_menusStack.pop();
		changeEngine<MenuEngine>(mousePosition, _cursor);
	}
}
void MenuEngine::backToGame()
{
	CLEAR_MENUS_STACK;
	_currMenu = &HierarchicalMenu::InGameMenu;
	changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
}
