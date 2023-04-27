#include "MenuEngine.h"
#include "AssetsManager.h"
#include "WindowManager.h"
#include "UIBaseButton.h"
#include "ClawLevelEngine.h"


#define DEFAULT_BG_IMAGE	"STATES/MENU/SCREENS/MENU.PCX"
#define MENU_ROOT			"STATES/MENU/IMAGES/MAIN/"
#define SINGLEPLAYER_ROOT	MENU_ROOT "SINGLEPLAYER/"


static const D2D1_SIZE_F DEFAULT_WINDOW_SIZE{ 800.f, 600.f };

MenuBackgroundImage::MenuBackgroundImage(string pcxPath)
	: UIBaseImage(*AssetsManager::loadImage(pcxPath)), _imgRatio(size.width / size.height) {}
void MenuBackgroundImage::Logic(uint32_t)
{
	// change image size according to window size:

	D2D1_SIZE_F wndSz = WindowManager::getRealSize();
	wndSz.height -= 40; // the title

	if (wndSz.width > wndSz.height)
	{
		size.height = wndSz.height;
		size.width = size.height * _imgRatio;
	}
	else if (wndSz.width < wndSz.height)
	{
		size.width = wndSz.width;
		size.height = size.width / _imgRatio;
	}

	position.x = wndSz.width / 2 - 5;
	position.y = wndSz.height / 2;
}


class HelpEngine : public MenuEngine
{
public:
	HelpEngine() : MenuEngine(false, "STATES/HELP/SCREENS/HELP.PCX") {}
	void OnKeyUp(int key) override { backToMenu(); }
	void OnMouseButtonUp(MouseButtons btn) override { backToMenu(); }
private:
	void backToMenu()
	{
		_currMenu = _menusStack.top();
		_menusStack.pop();
		changeEngine<MenuEngine>();
	}
};

class MenuItem : virtual UIBaseButton, virtual UIBaseImage
{
	using UIBaseImage::Draw;

public:
	MenuItem(string pcxPath, float xRatio, float yRatio, function<void(MouseButtons)> onClick, 
		MenuBackgroundImage* bgImg, MenuEngine* parent)
		: UIBaseButton(onClick, parent),
		UIBaseImage(*AssetsManager::loadImage(pcxPath)),
		_posRatio({ xRatio, yRatio }), _bgImg(bgImg)
	{
		_sizeRatio = { DEFAULT_WINDOW_SIZE.width / UIBaseImage::size.width, DEFAULT_WINDOW_SIZE.height / UIBaseImage::size.height };
	}

	void Logic(uint32_t) override
	{
		UIBaseButton::size.width = _bgImg->size.width / _sizeRatio.width;
		UIBaseButton::size.height = _bgImg->size.height / _sizeRatio.height;
		UIBaseButton::position.x = _bgImg->position.x + _posRatio.x * _bgImg->size.width;
		UIBaseButton::position.y = _bgImg->position.y + _posRatio.y * _bgImg->size.height;
		UIBaseImage::position = UIBaseButton::position;
		UIBaseImage::size = UIBaseButton::size;
		UIBaseButton::Logic(0);
	}

	// mul the size ratio by n
	void mulImageSizeRatio(float n)
	{
		_sizeRatio.width *= n;
		_sizeRatio.height *= n;
	}

private:
	D2D1_POINT_2F _posRatio;
	D2D1_SIZE_F _sizeRatio;
	MenuBackgroundImage* _bgImg;
};

struct HierarchicalMenu
{
	enum Commands : uint8_t
	{
		Nop,
		NotImpleted,
		MenuIn, // go to next sub-menu
		MenuOut, // back to previous menu
		ExitApp,
		Help,
		OpenLevel, // `OpenLevel | (<lvlNo> << 4)`

		OpenLevel_1 = OpenLevel | (0x10),
		OpenLevel_2 = OpenLevel | (0x20),
		OpenLevel_3 = OpenLevel | (0x30),
		OpenLevel_4 = OpenLevel | (0x40),
		OpenLevel_5 = OpenLevel | (0x50),
		OpenLevel_6 = OpenLevel | (0x60),
		OpenLevel_7 = OpenLevel | (0x70),
		OpenLevel_8 = OpenLevel | (0x80),
		OpenLevel_9 = OpenLevel | (0x90),
		OpenLevel_10 = OpenLevel | (0xA0),
		OpenLevel_11 = OpenLevel | (0xB0),
		OpenLevel_12 = OpenLevel | (0xC0),
		OpenLevel_13 = OpenLevel | (0xD0),
		OpenLevel_14 = OpenLevel | (0xE0)
	};

	HierarchicalMenu(string pcxPath, uint8_t cmd, float xRatio, float yRatio, vector<HierarchicalMenu> subMenus = {})
		: pcxPath(pcxPath), subMenus(subMenus), xRatio(xRatio), yRatio(yRatio), cmd(cmd) {}

	vector<HierarchicalMenu> subMenus;
	string pcxPath;
	float xRatio, yRatio;
	uint8_t cmd;

	static HierarchicalMenu MainMenu;
};

HierarchicalMenu HierarchicalMenu::MainMenu("", MenuIn, 0, 0, {
	HierarchicalMenu(MENU_ROOT "014_TITLE.PCX", Nop, 0, -0.15f),
	HierarchicalMenu(MENU_ROOT "001_SINGLEPLAYER.PCX", MenuIn, 0, -0.05f, {
		HierarchicalMenu(SINGLEPLAYER_ROOT "014_TITLE.PCX", Nop, 0, -0.15f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "001_NEWGAME.PCX", MenuIn, 0, -0.05f, {
			HierarchicalMenu(SINGLEPLAYER_ROOT "NEW/001_TITLE.PCX", Nop, 0, -0.15f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/001_LEVELONE.PCX",		OpenLevel_1, -0.2f, -0.05f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/004_LEVELTWO.PCX",		OpenLevel_2, -0.2f, 0.01f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/007_LEVELTHREE.PCX",		OpenLevel_3, -0.2f, 0.07f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/010_LEVELFOUR.PCX",		OpenLevel_4, -0.2f, 0.13f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/013_LEVELFIVE.PCX",		OpenLevel_5, -0.2f, 0.19f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/016_LEVELSIX.PCX",		OpenLevel_6, -0.2f, 0.25f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/019_LEVELSEVEN.PCX",		OpenLevel_7, -0.2f, 0.31f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/022_LEVELEIGHT.PCX",		OpenLevel_8, 0.2f, -0.05f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/025_LEVELNINE.PCX",		OpenLevel_9, 0.2f, 0.01f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/028_LEVELTEN.PCX",		OpenLevel_10, 0.2f, 0.07f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/031_LEVELELEVEN.PCX",	OpenLevel_11, 0.2f, 0.13f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/034_LEVELTWELVE.PCX",	OpenLevel_12, 0.2f, 0.19f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/037_LEVELTHIRTEEN.PCX",	OpenLevel_13, 0.2f, 0.25f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/040_LEVELFOURTEEN.PCX",	OpenLevel_14, 0.2f, 0.31f),
			HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/043_BACK.PCX", MenuOut, 0, 0.4f),
		}),
		HierarchicalMenu(SINGLEPLAYER_ROOT "003_LOADGAME.PCX", NotImpleted, 0, 0.04f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "005_LOADCUSTOM.PCX", NotImpleted, 0, 0.13f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "007_SAVEGAME.PCX", MenuIn, 0, 0.22f, {
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/003_EXPLANATION.PCX", Nop, 0, 0),
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/001_PREVIOUSMENU.PCX", MenuOut, 0, 0.22f),
		}),
		HierarchicalMenu(SINGLEPLAYER_ROOT "009_UPLOADSCORES.PCX", NotImpleted, 0, 0.31f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "012_PREVIOUSMENU.PCX", MenuOut, 0, 0.4f),
	}),
	HierarchicalMenu(MENU_ROOT "003_MULTIPLAYER.PCX", NotImpleted, 0, 0.03f),
	HierarchicalMenu(MENU_ROOT "005_REPLAYMOVIES.PCX", NotImpleted, 0, 0.11f),
	HierarchicalMenu(MENU_ROOT "008_OPTIONS.PCX", NotImpleted, 0, 0.19f),
	HierarchicalMenu(MENU_ROOT "015_CREDITS.PCX", NotImpleted, 0, 0.27f),
	HierarchicalMenu(MENU_ROOT "010_HELP.PCX", Help, 0, 0.35f),
	HierarchicalMenu(MENU_ROOT "012_QUIT.PCX", MenuIn, 0, 0.43f, {
		HierarchicalMenu(MENU_ROOT "QUIT/005_AREYOUSURE.PCX", Nop, 0, 0),
		HierarchicalMenu(MENU_ROOT "QUIT/001_YES.PCX", ExitApp, 0, 0.15f),
		HierarchicalMenu(MENU_ROOT "QUIT/003_NO.PCX", MenuOut, 0, 0.22f),
	})
});

stack<const HierarchicalMenu*> MenuEngine::_menusStack;
const HierarchicalMenu* MenuEngine::_currMenu = &HierarchicalMenu::MainMenu;

MenuEngine::MenuEngine(bool allocChildren, const string& bgPcxPath) : MenuEngine({}, nullptr, allocChildren, bgPcxPath) {}
MenuEngine::MenuEngine(D2D1_POINT_2U mPos, shared_ptr<Animation> cursor, bool allocChildren, const string& bgPcxPath)
{
	_elementsList.push_back(_bgImg = DBG_NEW MenuBackgroundImage(bgPcxPath.empty() ? DEFAULT_BG_IMAGE : bgPcxPath));

	mousePosition = mPos;
	_nextEngine = nullptr;
	WindowManager::setTitle("Claw");
	WindowManager::setWindowOffset(nullptr);

	if (!allocChildren)
	{
		return;
	}

	if (cursor)
	{
		_cursor = cursor;
	}
	else if (_currMenu == &HierarchicalMenu::MainMenu)
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

		case HierarchicalMenu::Help:
			onClick = [&](MouseButtons) {
				_menusStack.push(_currMenu);
				_currMenu = &m;
				changeEngine<HelpEngine>();
			};
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

		default:
			if ((m.cmd & HierarchicalMenu::OpenLevel) == HierarchicalMenu::OpenLevel)
			{
				onClick = [&](MouseButtons) {
					_currMenu = &m;
					while (_menusStack.size()) _menusStack.pop();
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
MenuEngine::~MenuEngine() {
	if (_cursor) _elementsList.pop_back(); // remove the cursor
	if (_bgImg) { _elementsList.erase(_elementsList.begin()); delete _bgImg; }// remove the bg image
	for (UIBaseElement* i : _elementsList) delete i;
}
void MenuEngine::Logic(uint32_t elapsedTime) {
	if (_cursor) _cursor->position = { mousePosition.x + 16.f, mousePosition.y + 17.f };
	BaseEngine::Logic(elapsedTime);
}


LevelLoadingEngine::LevelLoadingEngine(int lvlNo) :
	MenuEngine(false, "LEVEL" + to_string(lvlNo) + "/SCREENS/LOADING.PCX"),
	_totalTime(0), _lvlNo(lvlNo), _isDrawn(false) {}
void LevelLoadingEngine::Logic(uint32_t elapsedTime)
{
	_bgImg->Logic(elapsedTime);
	_totalTime += elapsedTime;
	if (_isDrawn && _totalTime > 100)
		changeEngine<ClawLevelEngine>(_lvlNo);
	_isDrawn = true; // After once it is sure to be drawn.
}


string getBGImgPath1(int l)
{
	string path = "STATES/BOOTY/SCREENS/";
	char num3[4]; sprintf(num3, "%03d", l * 2 - 1);
	path += num3;
	if (l % 2) path += "MAP";
	else path += "AMULET";
	path += ".PCX";
	return path;
}
string getBGImgPath2(int l)
{
	string path = "STATES/BOOTY/SCREENS/";
	char num3[4]; sprintf(num3, "%03d", l * 2);
	path += num3;
	if (l % 2) path += "MAP";
	else path += "AMULET";
	path += ".PCX";
	return path;
}


#define NUM_OF_TREASURES 9
#define type_Treasure_Skull		Item::Treasure_Skull_Blue
#define type_Treasure_Crowns	Item::Treasure_Crowns_Green
#define type_Treasure_Geckos	Item::Treasure_Geckos_Red
#define type_Treasure_Scepters	Item::Treasure_Scepters_Red
#define type_Treasure_Crosses	Item::Treasure_Crosses_Blue
#define type_Treasure_Chalices	Item::Treasure_Chalices_Green
#define type_Treasure_Rings		Item::Treasure_Rings_Purple
#define type_Treasure_Goldbars	Item::Treasure_Goldbars
#define type_Treasure_Coins		Item::Treasure_Coins


static const pair<Item::Type, const char*> treasuresData[NUM_OF_TREASURES] = {
	{ type_Treasure_Skull, "STATES/BOOTY/IMAGES/TREASURE/JEWELEDSKULL/BLUE/002.PID" },
	{ type_Treasure_Crowns, "STATES/BOOTY/IMAGES/TREASURE/CROWNS/GREEN/003.PID" },
	{ type_Treasure_Geckos, "STATES/BOOTY/IMAGES/TREASURE/GECKOS/RED/001.PID" },
	{ type_Treasure_Scepters, "STATES/BOOTY/IMAGES/TREASURE/SCEPTERS/RED/001.PID" },
	{ type_Treasure_Crosses, "STATES/BOOTY/IMAGES/TREASURE/CROSSES/BLUE/002.PID" },
	{ type_Treasure_Chalices, "STATES/BOOTY/IMAGES/TREASURE/CHALICES/GREEN/003.PID" },
	{ type_Treasure_Rings, "STATES/BOOTY/IMAGES/TREASURE/RINGS/PURPLE/004.PID" },
	{ type_Treasure_Goldbars, "STATES/BOOTY/IMAGES/TREASURE/GOLDBARS/001.PID" },
	{ type_Treasure_Coins, "STATES/BOOTY/IMAGES/TREASURE/COINS/001.PID" }
};

static const char* const scorenumbersPaths[] = {
	"STATES/BOOTY/IMAGES/SCORENUMBERS/000.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/001.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/002.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/003.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/004.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/005.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/006.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/007.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/008.PID",
	"STATES/BOOTY/IMAGES/SCORENUMBERS/009.PID"
};


// TODO: draw cool animation of map/gem before showing score

LevelEndEngine::LevelEndEngine(int lvlNum, map<Item::Type, uint32_t> collectedTreasures)
	: MenuEngine(false, getBGImgPath1(lvlNum)), _lvlNum(lvlNum), _state(Start)
{
	WindowManager::setBackgroundColor(ColorF::Black);

	for (const auto& i : collectedTreasures)
	{
		Item::Type tresType;

		switch (i.first)
		{
		case Item::Default:
		case Item::Treasure_Coins:
			tresType = type_Treasure_Coins;
			break;

		case Item::Treasure_Goldbars:
			tresType = type_Treasure_Goldbars;
			break;

		case Item::Treasure_Rings_Red:
		case Item::Treasure_Rings_Green:
		case Item::Treasure_Rings_Blue:
		case Item::Treasure_Rings_Purple:
			tresType = type_Treasure_Rings;
			break;

		case Item::Treasure_Necklace:
		case Item::Treasure_Chalices_Red:
		case Item::Treasure_Chalices_Green:
		case Item::Treasure_Chalices_Blue:
		case Item::Treasure_Chalices_Purple:
			tresType = type_Treasure_Chalices;
			break;

		case Item::Treasure_Crosses_Red:
		case Item::Treasure_Crosses_Green:
		case Item::Treasure_Crosses_Blue:
		case Item::Treasure_Crosses_Purple:
			tresType = type_Treasure_Crosses;
			break;

		case Item::Treasure_Scepters_Red:
		case Item::Treasure_Scepters_Green:
		case Item::Treasure_Scepters_Blue:
		case Item::Treasure_Scepters_Purple:
			tresType = type_Treasure_Scepters;
			break;

		case Item::Treasure_Geckos_Red:
		case Item::Treasure_Geckos_Green:
		case Item::Treasure_Geckos_Blue:
		case Item::Treasure_Geckos_Purple:
			tresType = type_Treasure_Geckos;
			break;

		case Item::Treasure_Crowns_Red:
		case Item::Treasure_Crowns_Green:
		case Item::Treasure_Crowns_Blue:
		case Item::Treasure_Crowns_Purple:
			tresType = type_Treasure_Crowns;
			break;

		case Item::Treasure_Skull_Red:
		case Item::Treasure_Skull_Green:
		case Item::Treasure_Skull_Blue:
		case Item::Treasure_Skull_Purple:
			tresType = type_Treasure_Skull;
			break;

		default:
			throw Exception("invalid treasure type");
		}

		_collectedTreasures[tresType] += i.second;
	}
}
void LevelEndEngine::Logic(uint32_t elapsedTime)
{
	MenuEngine::Logic(elapsedTime);

	switch (_state)
	{
	case Start:
		break;

	case DrawScore:
		delete _bgImg;
		_elementsList.clear();
		_elementsList.push_back(_bgImg = DBG_NEW MenuBackgroundImage(getBGImgPath2(_lvlNum)));
		_state += 1;

		// TODO: draw all treasures and their points

		int digits[3]; // we have only 3 digits to display
		MenuItem* item;
		float x, y;
		int i, j;

		for (i = 0; i < NUM_OF_TREASURES; i++)
		{
			x = -0.18f;
			y = (-230 + 53 * i) / 600.f;

			// draw current treasures
			item = DBG_NEW MenuItem(treasuresData[i].second, -0.25f, y, {}, _bgImg, this);
			item->mulImageSizeRatio(0.75f);
			_elementsList.push_back((UIBaseImage*)item);

			// draw collected treasures count
			digits[0] = _collectedTreasures[treasuresData[i].first] / 100;
			digits[1] = _collectedTreasures[treasuresData[i].first] / 10 % 10;
			digits[2] = _collectedTreasures[treasuresData[i].first] % 10;
			
			for (j = 0; j < 3; j++)
			{
				item = DBG_NEW MenuItem(scorenumbersPaths[digits[j]], x, y, {}, _bgImg, this);
				item->mulImageSizeRatio(0.75f);
				_elementsList.push_back((UIBaseImage*)item);
				x += 0.02f;
			}

			// TODO: now it <IMG> <amount>. need continue to: <IMG> <amount> OF <total> X<score> = <total_treasure_score>
		}

		break;

	case Wait:
		// nop
		break;

	case End:
		playNextLevel();
		break;

	default:
		break;
	}
}
void LevelEndEngine::OnKeyUp(int key) { _state += 1; }
void LevelEndEngine::OnMouseButtonUp(MouseButtons btn) { _state += 1; }
void LevelEndEngine::playNextLevel()
{
	if (_lvlNum == 14) // last level
		changeEngine<MenuEngine>(); // TODO: go to credits
	else
		changeEngine<LevelLoadingEngine>(_lvlNum + 1);
}
