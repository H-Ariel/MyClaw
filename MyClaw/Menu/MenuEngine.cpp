#include "MenuEngine.h"
#include "../Assets-Managers/AssetsManager.h"
#include "GameEngine/WindowManager.h"
#include "../ClawLevelEngine.h"
#include "HierarchicalMenu.h"
#include "HelpScreenEngine.h"
#include "CreditsEngine.h"
#include "LevelLoadingEngine.h"
#include "MenuItem.h"


stack<const HierarchicalMenu*> MenuEngine::_menusStack;
const HierarchicalMenu* MenuEngine::_currMenu = &HierarchicalMenu::MainMenu;


// TODO: combine with SavedDataManager (1 class and 1 file for setting and saved game)
class SettingsManager
{
	const char* settingsFile = "Claw.settings";

public:
	SettingsManager() { settings = SavedDataManager::loadSettings(); }
	~SettingsManager() { SavedDataManager::saveSettings(settings); }

	void setDetails(bool d) { settings.details = d; }
	void setFrontLayer(bool f) { settings.frontLayer = f; }
	void setMovies(bool m) { settings.movies = m; }

	void setSoundVolume(int v) { settings.soundVolume = v; }
	void setVoice(bool v) { settings.voice = v; }
	void setAmbient(bool a) { settings.ambient = a; }
	void setMusicVolume(int v) { settings.musicVolume = v; }

	bool getDetails() const { return settings.details; }
	bool getFrontLayer() const { return settings.frontLayer; }
	bool getMovies() const { return settings.movies; }
	int8_t getSoundVolume() const { return settings.soundVolume; }
	bool getVoice() const { return settings.voice; }
	bool getAmbient() const { return settings.ambient; }
	int8_t getMusicVolume() const { return settings.musicVolume; }

	void switchDetails() { settings.details = !settings.details; }
	void switchFrontLayer() { settings.frontLayer = !settings.frontLayer; }
	void switchMovies() { settings.movies = !settings.movies; }
	void switchSoundVolume() { settings.soundVolume = (settings.soundVolume == 0) ? 100 : 0; }
	void switchVoice() { settings.voice = !settings.voice; }
	void switchAmbient() { settings.ambient = !settings.ambient; }
	void switchMusicVolume() { settings.musicVolume = (settings.musicVolume == 0) ? 100 : 0; }

private:
	SavedDataManager::SettingsData settings;
};

static SettingsManager settingsManager;


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

	function<void(MenuItem*)> onClick;

	// when the menu is SelectLevelMenu we have two columns of buttons
	const bool isSelectLevelMenu = _currMenu == &HierarchicalMenu::SelectLevelMenu;
	float xRatio = 0, yRatio = float(isSelectLevelMenu ? _currMenu->subMenus.size() - 7 : _currMenu->subMenus.size()) / -50;
	float firstYRatio = yRatio;
	int buttonsCount = 0;
	int initialState = 0;

	for (const HierarchicalMenu& m : _currMenu->subMenus)
	{
		initialState = 0;

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
				if (endsWith(m.pcxPath, "001.PCX")) dir = "NEW";
				else if (endsWith(m.pcxPath, "003.PCX")) dir = "LOAD";
				//else throw Exception("invalid pcx path: " + m.pcxPath);
				HierarchicalMenu::SelectLevelMenu.subMenus[0].pcxPath = SINGLEPLAYER_ROOT + dir + "/001.PCX";

#ifndef _DEBUG // in debug mode, all levels are available for me :)
				// check which levels are available and change the menu-buttons accordingly
				for (int i = 2; i <= 14; i++) // i=2 because level 1 must be available by default
				{
					int n = i * 3;
					if (SavedDataManager::canLoadGame(i, 0))
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
					HierarchicalMenu::SelectLevelMenu.subMenus[i].pcxPath = path;
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

		case HierarchicalMenu::Help:
			onClick = [&](MenuItem*) {
				_menusStack.push(_currMenu);
				_currMenu = &m;
				changeEngine<HelpScreenEngine>();
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
			initialState = settingsManager.getDetails();

			onClick = [&](MenuItem* item) {
				item->state = (item->state == 0) ? 1 : 0;
				settingsManager.switchDetails();
			};
			break;

		case HierarchicalMenu::FrontLayer:
			initialState = settingsManager.getFrontLayer();

			onClick = [&](MenuItem* item) {
				item->state = (item->state == 0) ? 1 : 0;
				settingsManager.switchFrontLayer();
			};
			break;

		case HierarchicalMenu::Movies:
			initialState = settingsManager.getMovies();

			onClick = [&](MenuItem* item) {
				item->state = (item->state == 0) ? 1 : 0;
				settingsManager.switchMovies();
			};
			break;

		case HierarchicalMenu::Sound:
			initialState = settingsManager.getSoundVolume() != 0;

			onClick = [&](MenuItem* item) {
				item->state = (item->state == 0) ? 1 : 0;
				settingsManager.switchSoundVolume();
			};
		break;

		case HierarchicalMenu::Voice:
			initialState = settingsManager.getVoice();

			onClick = [&](MenuItem* item) {
				item->state = (item->state == 0) ? 1 : 0;
				settingsManager.switchVoice();
			};
			break;

		case HierarchicalMenu::Ambient:
			initialState = settingsManager.getAmbient();

			onClick = [&](MenuItem* item) {
				item->state = (item->state == 0) ? 1 : 0;
				settingsManager.switchAmbient();
			};
			break;

		case HierarchicalMenu::Music:
			initialState = settingsManager.getMusicVolume() != 0;

			onClick = [&](MenuItem* item) {
				item->state = (item->state == 0) ? 1 : 0;
				settingsManager.switchMusicVolume();
			};
			break;

		default:
			// here we play with bits, strings, and paths to get the level number and checkpoint number :)

			if ((m.cmd & HierarchicalMenu::LoadCheckpoint) == HierarchicalMenu::LoadCheckpoint)
			{
				onClick = [&](MenuItem*) {
					int level = stoi(HierarchicalMenu::SelectCheckpoint.subMenus[0]
						.pcxPath.substr(strlen(LOAD_CHECKPOINT_ROOT), 3)) - 10;
					int checkpoint = ((m.cmd & 0xf0) >> 4) - 1;
					changeEngine<LevelLoadingEngine>(level, checkpoint);
				};
			}
			else if ((m.cmd & HierarchicalMenu::OpenLevel) == HierarchicalMenu::OpenLevel)
			{
				onClick = [&](MenuItem*) {
					int level = (m.cmd & 0xf0) >> 4;

					if (contains(_currMenu->subMenus[0].pcxPath, "NEW")) // new game
					{
						changeEngine<LevelLoadingEngine>(level);
					}
					else if (contains(_currMenu->subMenus[0].pcxPath, "LOAD")) // load checkpoint
					{
						// set title:
						char path[64] = {};
						sprintf(path, LOAD_CHECKPOINT_ROOT "%03d.PCX", level + 10);
						HierarchicalMenu::SelectCheckpoint.subMenus[0].pcxPath = path;

						// check which checkpoints are available and change the menu-buttons accordingly
						if (SavedDataManager::canLoadGame(level, 1))
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[2].cmd = HierarchicalMenu::LoadCheckpoint_1;
							HierarchicalMenu::SelectCheckpoint.subMenus[2].pcxPath = LOAD_CHECKPOINT_ROOT "003.PCX";
						}
						else
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[2].cmd = HierarchicalMenu::Nop;
							HierarchicalMenu::SelectCheckpoint.subMenus[2].pcxPath = LOAD_CHECKPOINT_ROOT "005.PCX";
						}
						if (SavedDataManager::canLoadGame(level, 2))
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[3].cmd = HierarchicalMenu::LoadCheckpoint_2;
							HierarchicalMenu::SelectCheckpoint.subMenus[3].pcxPath = LOAD_CHECKPOINT_ROOT "006.PCX";
						}
						else
						{
							HierarchicalMenu::SelectCheckpoint.subMenus[3].cmd = HierarchicalMenu::Nop;
							HierarchicalMenu::SelectCheckpoint.subMenus[3].pcxPath = LOAD_CHECKPOINT_ROOT "008.PCX";
						}

						menuIn(&HierarchicalMenu::SelectCheckpoint);
					}
				};
			}
			else onClick = [](MenuItem*) { MessageBoxA(nullptr, "not impleted (default case)", "", 0); };

			break;
		}

		if (isSelectLevelMenu) { // find `xRatio` for title / columns
			if (buttonsCount == 0 || buttonsCount == 15) xRatio = 0;
			else if (buttonsCount <= 7) xRatio = -0.2f;
			else xRatio = 0.2f;
			if (buttonsCount == 8) yRatio = firstYRatio;
			buttonsCount++;
		}
   
		MenuItem* itm = DBG_NEW MenuItem(m.pcxPath, m.markedPcxPath, 
			m.pcxPath2, m.markedPcxPath2,
			xRatio, yRatio, onClick, _bgImg, this, initialState);
		itm->mulImageSizeRatio(1.25f); // magic number to fit image to screen size
		_elementsList.push_back(itm);
		yRatio += itm->size.height / _bgImg->size.height / 2 + 0.04f;

		if (isSelectLevelMenu && buttonsCount == 1)
			firstYRatio = yRatio; // find the height of the first button (so columns are aligned)
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
	//if (_cursor) _cursor->position = { (float)mousePosition.x + 28.f , (float)mousePosition.y + 28 };

	ScreenEngine::Logic(elapsedTime);

	// check if cursor collide with any button
	for (UIBaseElement* e : _elementsList)
	{
		if (isinstance<MenuItem>(e) && e->GetRect().intersects(_cursor->GetRect()))
		{
			_currMarkedItem = (MenuItem*)e;
			if (!_currMarkedItem->isActive())
				_currMarkedItem = nullptr;
			break;
		}
	}

	for (UIBaseElement* e : _elementsList)
	{
		if (isinstance<MenuItem>(e))
		{
			if (MenuItem* itm = (MenuItem*)e)
				itm->marked = (itm == _currMarkedItem);
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
			if (key == VK_DOWN)
				_currMarkedItem = (MenuItem*)_elementsList[2];
			else if (key == VK_UP)
				_currMarkedItem = (MenuItem*)_elementsList[_elementsList.size() - 1];
		}

		_currMarkedItem->marked = true;
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
