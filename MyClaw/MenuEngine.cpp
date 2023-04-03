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

shared_ptr<Animation> getSpecialItem(int l)
{
	shared_ptr<Animation> ani;

	char imagesPath[38];
	sprintf(imagesPath, "STATES/BOOTY/IMAGES/MAPPIECE/LEVEL%03d", l);

	switch (l)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 9:
	case 11: {
		// map
		ani = AssetsManager::createAnimationFromDirectory(imagesPath, 125, false);
	}	break;

	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
		// gem
		ani = AssetsManager::loadAnimation("STATES/BOOTY/ANIS/GEM" + to_string(l) + ".ANI", imagesPath);
		break;

	case 13:
		// has 2 gems...
		break;

	default:
		throw Exception("invalid level number");
	}

	return ani;
}
D2D1_POINT_2F getItemEndPosition(int l)
{
	switch (l)
	{
	case 1: return { 0.38f, 0.32f };

	default: throw Exception("invalid level number");
	}
	return {};
}




LevelEndEngine::LevelEndEngine(int lvlNum)
	: MenuEngine(false, getBGImgPath1(lvlNum)), _lvlNum(lvlNum), _state(Start), _levelSpecialItem(getSpecialItem(lvlNum))
{
	// TODO: set the palette

	WindowManager::setBackgroundColor(ColorF::Black);

//	_itemEndPos = getItemEndPosition(lvlNum);
	_itemEndPos = {};
}
void LevelEndEngine::Logic(uint32_t elapsedTime)
{
	MenuEngine::Logic(elapsedTime);

	_levelSpecialItem->position.x = _bgImg->position.x + _itemEndPos.x * _bgImg->size.width;
	_levelSpecialItem->position.y = _bgImg->position.y + _itemEndPos.y * _bgImg->size.height;

	switch (_state)
	{
	case Start:
		break;

	case InsertMap:
		// TODO: cool animation of map piece
	//	_elementsList.push_back(_levelSpecialItem.get());
		_state = DrawDots;
		break;

	case DrawDots:
		// TODO: draw dots
		break;

	case DrawGem:
		// TODO: cool animation of gem
		break;

	case DrawScore:
		delete _bgImg;
		_elementsList.clear();
		_elementsList.push_back(_bgImg = DBG_NEW MenuBackgroundImage(getBGImgPath2(_lvlNum)));
		_state += 1;
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
// TODO: continue
