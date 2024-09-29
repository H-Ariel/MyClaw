#include "MenuEngine.h"
#include "GameEngine/WindowManager.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../ClawLevelEngine.h"
#include "HierarchicalMenu.h"
#include "CreditsEngine.h"
#include "LevelLoadingEngine.h"
#include "MenuItem.h"


stack<const HierarchicalMenu*> MenuEngine::_menusStack;
const HierarchicalMenu* MenuEngine::_currMenu = &HierarchicalMenu::MainMenu;


MenuEngine::MenuEngine(const string& bgPcxPath) : MenuEngine({}, nullptr, bgPcxPath) {}
MenuEngine::MenuEngine(D2D1_POINT_2U mPos, shared_ptr<UIAnimation> cursor, const string& bgPcxPath)
	: ScreenEngine(bgPcxPath), _currMarkedItem(nullptr)
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

	function<void(MenuItem*)> onClick; // for buttons
	function<void(MenuSlider*, int)> onMove; // for sliders

	const bool isSelectLevelMenu = _currMenu == &HierarchicalMenu::SelectLevelMenu; // when the menu is SelectLevelMenu we have two columns of buttons
	const bool isHelp = _currMenu->subMenus[0].imagePath == "STATES/HELP/SCREENS/HELP.PCX"; // for help screen we do not have offset
	float xRatio = 0;
	float yRatio = float(isHelp ? 0 : isSelectLevelMenu ? _currMenu->subMenus.size() - 7 : _currMenu->subMenus.size()) / -48;
	float firstYRatio = yRatio;
	int buttonsCount = 0;
	int initialValue = 0;
	bool isSlider = false;

	for (const HierarchicalMenu& m : _currMenu->subMenus)
	{
		initialValue = 0;
		isSlider = false;

		switch (m.cmd)
		{
		case HierarchicalMenu::Nop:
			onClick = nullptr;
			break;

		case HierarchicalMenu::NotImpleted:
			onClick = [](MenuItem*) { MessageBoxA(nullptr, "not impleted", "", 0); };
			break;

		case HierarchicalMenu::MenuIn:
			onClick = [&](MenuItem*) { menuIn(&m); };
			break;

		case HierarchicalMenu::MenuOut:
			onClick = [&](MenuItem*) { menuOut(); };
			break;

		case HierarchicalMenu::SelectLevel:
			onClick = [&](MenuItem*) {
				// change the title image:
				string dir = "";
				if (endsWith(m.imagePath, "001.PCX")) dir = "NEW";
				else if (endsWith(m.imagePath, "003.PCX")) dir = "LOAD";
				//else throw Exception("invalid pcx path: " + m.imagePath);
				HierarchicalMenu::SelectLevelMenu.subMenus[0].imagePath = SINGLEPLAYER_ROOT + dir + "/001.PCX";

#ifndef _DEBUG // in debug mode, all levels are available for me :)
				// check which levels are available and change the menu-buttons accordingly
				for (int i = 1; i <= 14; i++)
				{
					int n = i * 3;
					if (SavedDataManager::instance.canLoadGame(i, 0))
					{
						HierarchicalMenu::SelectLevelMenu.subMenus[i].cmd = HierarchicalMenu::OpenLevel | (i << 4);
						n -= 2;
					}
					else
					{
						HierarchicalMenu::SelectLevelMenu.subMenus[i].cmd = HierarchicalMenu::Nop;
					}

					char path[52] = {};
					sprintf(path, SINGLEPLAYER_ROOT "LEVELS/%03d.PCX", n);
					HierarchicalMenu::SelectLevelMenu.subMenus[i].imagePath = path;
				}
#endif

				menuIn(&HierarchicalMenu::SelectLevelMenu);
			};
			break;

		case HierarchicalMenu::ExitApp:
			onClick = [&](MenuItem*) {
				_nextEngine = nullptr;
				stopEngine = true;
			};
			break;

		case HierarchicalMenu::Credits:
			onClick = [&](MenuItem*) {
				_menusStack.push(_currMenu);
				_currMenu = &m;
				changeEngine<CreditsEngine>();
			};
			break;

		case HierarchicalMenu::BackToGame:
			onClick = [&](MenuItem*) { backToGame(); };
			break;

		case HierarchicalMenu::EndLife:
			onClick = [&](MenuItem*) {
				BasePlaneObject::player->endLife();
				clearMenusStack();
				_currMenu = &HierarchicalMenu::InGameMenu;
				changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
			};
			break;

		case HierarchicalMenu::EndGame:
			onClick = [&](MenuItem*) {
				AssetsManager::clearLevelAssets(_clawLevelEngineFields->_wwd->levelNumber);
				clearMenusStack();
				_currMenu = &HierarchicalMenu::MainMenu;
				_clawLevelEngineFields.reset();
				changeEngine<MenuEngine>();
				BasePlaneObject::player = nullptr; // do not recycle the player in new game
			};
			break;

		case HierarchicalMenu::Details:
			initialValue = SavedDataManager::instance.settings.details;
			onClick = [&](MenuItem* item) {
				item->switchState();
				SavedDataManager::instance.settings.details = !SavedDataManager::instance.settings.details;
			};
			break;

		case HierarchicalMenu::FrontLayer:
			initialValue = SavedDataManager::instance.settings.frontLayer;
			onClick = [&](MenuItem* item) {
				item->switchState();
				SavedDataManager::instance.settings.frontLayer = !SavedDataManager::instance.settings.frontLayer;
			};
			break;

		case HierarchicalMenu::Area:
			isSlider = true;
			initialValue = SavedDataManager::instance.settings.area;
			onMove = [&](MenuSlider* slider, int value) {
				SavedDataManager::instance.settings.area = value;
			};
			break;

		case HierarchicalMenu::Movies:
			initialValue = SavedDataManager::instance.settings.movies;
			onClick = [&](MenuItem* item) {
				item->switchState();
				SavedDataManager::instance.settings.movies = !SavedDataManager::instance.settings.movies;
			};
			break;

		case HierarchicalMenu::Sound:
			isSlider = true;
			initialValue = SavedDataManager::instance.settings.soundVolume;
			onMove = [&](MenuSlider* slider, int value) {
				slider->setStates(value == 0);
				SavedDataManager::instance.settings.soundVolume = value;
			};
			break;

		case HierarchicalMenu::Voice:
			initialValue = SavedDataManager::instance.settings.voice;
			onClick = [&](MenuItem* item) {
				item->switchState();
				SavedDataManager::instance.settings.voice = !SavedDataManager::instance.settings.voice;
			};
			break;

		case HierarchicalMenu::Ambient:
			initialValue = SavedDataManager::instance.settings.ambient;
			onClick = [&](MenuItem* item) {
				item->switchState();
				SavedDataManager::instance.settings.ambient = !SavedDataManager::instance.settings.ambient;
			};
			break;

		case HierarchicalMenu::Music:
			isSlider = true;
			initialValue = SavedDataManager::instance.settings.musicVolume;
			onMove = [&](MenuSlider* slider, int value) {
				slider->setStates(value == 0);
				SavedDataManager::instance.settings.musicVolume = value;
			};
			break;

		default:
			// here we play with bits, strings, and paths to get the level number and checkpoint number :)

			if ((m.cmd & HierarchicalMenu::LoadCheckpoint) == HierarchicalMenu::LoadCheckpoint)
			{
				onClick = [&](MenuItem*) {
					int level = stoi(HierarchicalMenu::SelectCheckpoint.subMenus[0]
						.imagePath.substr(strlen(LOAD_CHECKPOINT_ROOT), 3)) - 10;
					int checkpoint = ((m.cmd & 0xf0) >> 4) - 1;
					changeEngine<LevelLoadingEngine>(level, checkpoint);
				};
			}
			else if ((m.cmd & HierarchicalMenu::OpenLevel) == HierarchicalMenu::OpenLevel)
			{
				onClick = [&](MenuItem*) {
					int level = (m.cmd & 0xf0) >> 4;

					if (contains(_currMenu->subMenus[0].imagePath, "NEW")) // new game
					{
						changeEngine<LevelLoadingEngine>(level);
					}
					else if (contains(_currMenu->subMenus[0].imagePath, "LOAD")) // load checkpoint
					{
						// set title:
						char path[64] = {};
						sprintf(path, LOAD_CHECKPOINT_ROOT "%03d.PCX", level + 10);
						HierarchicalMenu::SelectCheckpoint.subMenus[0].imagePath = path;

						// check which checkpoints are available and change the menu-buttons accordingly
						if (SavedDataManager::instance.canLoadGame(level, 1))
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[2].cmd = HierarchicalMenu::LoadCheckpoint_1;
							HierarchicalMenu::SelectCheckpoint.subMenus[2].imagePath = LOAD_CHECKPOINT_ROOT "003.PCX";
						}
						else
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[2].cmd = HierarchicalMenu::Nop;
							HierarchicalMenu::SelectCheckpoint.subMenus[2].imagePath = LOAD_CHECKPOINT_ROOT "005.PCX";
						}
						if (SavedDataManager::instance.canLoadGame(level, 2))
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[3].cmd = HierarchicalMenu::LoadCheckpoint_2;
							HierarchicalMenu::SelectCheckpoint.subMenus[3].imagePath = LOAD_CHECKPOINT_ROOT "006.PCX";
						}
						else
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[3].cmd = HierarchicalMenu::Nop;
							HierarchicalMenu::SelectCheckpoint.subMenus[3].imagePath = LOAD_CHECKPOINT_ROOT "008.PCX";
						}

						menuIn(&HierarchicalMenu::SelectCheckpoint);
					}
				};
			}
			else onClick = nullptr;

			break;
		}

		if (isSelectLevelMenu) { // find `xRatio` for title / columns
			if (buttonsCount == 0 || buttonsCount == 15) xRatio = 0;
			else if (buttonsCount <= 7) xRatio = -0.22f;
			else xRatio = 0.22f;
			if (buttonsCount == 8) yRatio = firstYRatio;
			buttonsCount++;
		}
   
		MenuItem* itm =
			isSlider 
			? DBG_NEW MenuSlider(m.imagePath, m.markedImagePath, m.toggleStateImage, m.markedToggleStateImage,
				xRatio, yRatio, onMove, _bgImg, this, initialValue)
			: DBG_NEW MenuItem(m.imagePath, m.markedImagePath, m.toggleStateImage, m.markedToggleStateImage,
				xRatio, yRatio, onClick, _bgImg, this, initialValue);

		_elementsList.push_back(itm);
		yRatio += itm->size.height / _bgImg->size.height / 2 + 0.048f;

		if (isSelectLevelMenu && buttonsCount == 1)
			firstYRatio = yRatio; // find the height of the first button (so columns are aligned)
	}

	if (_cursor)
	{
		_elementsList.push_back(_cursor.get());
	}

	// set _currMarkedItem to the first button
	for (UIBaseElement* e : _elementsList)
	{
		if (MenuItem* i = dynamic_cast<MenuItem*>(e))
		{
			if (i->isActive())
			{
				_currMarkedItem = i;
				_currMarkedItem->marked = true;
				break;
			}
		}
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
	//if (_cursor) _cursor->position = { (float)mousePosition.x + 28.f , (float)mousePosition.y + 28 };

	ScreenEngine::Logic(elapsedTime);

	// check if cursor collide with any button
	for (UIBaseElement* e : _elementsList) {
		if (isinstance<MenuItem>(e) && e->GetRect().intersects(_cursor->GetRect())) {
			if (((MenuItem*)e)->isActive()) {
				_currMarkedItem->marked = false;
				_currMarkedItem = (MenuItem*)e;
				_currMarkedItem->marked = true;
			}
			break;
		}
	}
}
void MenuEngine::OnKeyUp(int key)
{
	if (key == VK_ESCAPE)
	{
		if (_currMenu == &HierarchicalMenu::MainMenu) // from main-menu to quit-menu
			menuIn(&HierarchicalMenu::MainMenu.subMenus[7]);
		else if (_currMenu == &HierarchicalMenu::InGameMenu) // from in-game-menu back to game
			backToGame();
		else
			menuOut(); // back to previous menu
	}
	else if (key == VK_RETURN && _currMarkedItem)
	{
		_currMarkedItem->onClick(MouseButtons::Left);
	}
	else if (key == VK_UP || key == VK_DOWN)
	{
		MenuItem* prev = _currMarkedItem;

		if (_currMarkedItem)
		{
			auto it = find(_elementsList.begin(), _elementsList.end(), _currMarkedItem);
			if (it != _elementsList.end()) // if _currMarkedItem is null, it is not in the list
			{
				/*
				The elements list is like this:
				- background image
				- title image (except for InGameMenu)
				- button 1
				- button 2
				- ...
				- cursor
				*/

				MenuItem* itm = nullptr;
				const size_t minIdx = (_currMenu == &HierarchicalMenu::InGameMenu) ? 1 : 2;
				const size_t maxIdx = _elementsList.size() - 2;
				size_t idx = it - _elementsList.begin();

				if (key == VK_UP)
				{
					do {
						itm = (MenuItem*)(idx == minIdx ? _elementsList[maxIdx] : _elementsList[idx - 1]);
						idx = (idx == minIdx ? maxIdx : idx - 1);
					} while (!itm->isActive());
				}
				else if (key == VK_DOWN)
				{
					do {
						itm = (MenuItem*)(idx == maxIdx ? _elementsList[minIdx] : _elementsList[idx + 1]);
						idx = (idx == maxIdx ? minIdx : idx + 1);
					} while (!itm->isActive());
				}

				_currMarkedItem = itm;
			}
		}
		else
		{
			// TODO: search using loop and isActive (not indexes)
			if (key == VK_DOWN)
				_currMarkedItem = (MenuItem*)_elementsList[2];
			else if (key == VK_UP)
				_currMarkedItem = (MenuItem*)_elementsList[_elementsList.size() - 1];
		}

		if (_currMarkedItem != prev) {
			if (prev) prev->marked = false;
			_currMarkedItem->marked = true;
		}
	}
	else if (key == VK_LEFT || key == VK_RIGHT)
	{
		if (isinstance<MenuSlider>(_currMarkedItem))
		{
			((MenuSlider*)_currMarkedItem)->moveSlider(key == VK_RIGHT ? 1 : -1);
		}
	}
}

void MenuEngine::clearMenusStack()
{
	for (; _menusStack.size(); _menusStack.pop());
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
	clearMenusStack();
	_currMenu = &HierarchicalMenu::InGameMenu;
	changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
}
