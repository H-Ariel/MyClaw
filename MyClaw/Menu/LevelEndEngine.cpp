#include "LevelEndEngine.h"
#include "LevelLoadingEngine.h"
#include "CreditsEngine.h"
#include "MenuItem.h"
#include "../SavedDataManager.h"
#include "../Player.h"


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
	{ type_Treasure_Skull,		"STATES/BOOTY/IMAGES/TREASURE/JEWELEDSKULL/BLUE/002.PID" },
	{ type_Treasure_Crowns,		"STATES/BOOTY/IMAGES/TREASURE/CROWNS/GREEN/003.PID" },
	{ type_Treasure_Geckos,		"STATES/BOOTY/IMAGES/TREASURE/GECKOS/RED/001.PID" },
	{ type_Treasure_Scepters,	"STATES/BOOTY/IMAGES/TREASURE/SCEPTERS/RED/001.PID" },
	{ type_Treasure_Crosses,	"STATES/BOOTY/IMAGES/TREASURE/CROSSES/BLUE/002.PID" },
	{ type_Treasure_Chalices,	"STATES/BOOTY/IMAGES/TREASURE/CHALICES/GREEN/003.PID" },
	{ type_Treasure_Rings,		"STATES/BOOTY/IMAGES/TREASURE/RINGS/PURPLE/004.PID" },
	{ type_Treasure_Goldbars,	"STATES/BOOTY/IMAGES/TREASURE/GOLDBARS/001.PID" },
	{ type_Treasure_Coins,		"STATES/BOOTY/IMAGES/TREASURE/COINS/001.PID" }
};

char ScoreNumberPath[] = "STATES/BOOTY/IMAGES/SCORENUMBERS/00#.PID"; // base path for score numbers. # should be digit to display.


static string getBGImgPath(int img)
{
	char path[30];
	sprintf(path, "STATES/BOOTY/SCREENS/%03d.PCX", img);
	return path;
}

// TODO: draw cool animation of map/gem before showing score

LevelEndEngine::LevelEndEngine(int lvlNum, const map<Item::Type, uint32_t>& collectedTreasures)
	: ScreenEngine(getBGImgPath(lvlNum * 2 - 1)), _lvlNum(lvlNum), _state(Start)
{
	LogFile::log(LogFile::Info, "end level %d", lvlNum);

	if (_lvlNum != 14)
	{
		// save as checkpoint:
		SavedDataManager::GameData data = BasePlaneObject::player->getGameData();
		data.level = _lvlNum + 1;
		data.savePoint = SavedDataManager::SavePoints::Start;
		SavedDataManager::instance.saveGame(data);
	}

	_clawLevelEngineFields.reset(); // clear level data (if player enter menu from level)
	AssetsManager::clearLevelAssets();

	for (auto& i : collectedTreasures)
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
	ScreenEngine::Logic(elapsedTime);

	uint8_t digits[3] = {}; // we have only 3 digits to display
	float x, y;
	int i, j;

	switch (_state)
	{
	case DrawScore:
		delete _bgImg;
		_elementsList.clear();
		_elementsList.push_back(_bgImg = DBG_NEW MenuBackgroundImage(getBGImgPath(_lvlNum * 2)));
		_state += 1;

		y = -0.38f;

		// draw all treasures and their points
		for (i = 0; i < NUM_OF_TREASURES; i++)
		{
			x = -0.18f;

			// draw current treasures
			_elementsList.push_back(DBG_NEW MenuItem(treasuresData[i].second, -0.25f, y, _bgImg, this));

			// draw collected treasures count
			digits[0] = _collectedTreasures[treasuresData[i].first] / 100;
			digits[1] = _collectedTreasures[treasuresData[i].first] / 10 % 10;
			digits[2] = _collectedTreasures[treasuresData[i].first] % 10;

			for (j = 0; j < 3; j++)
			{
				ScoreNumberPath[35] = digits[j] + '0';
				_elementsList.push_back(DBG_NEW MenuItem(ScoreNumberPath, x, y, _bgImg, this));
				x += 0.02f;
			}

			y += 0.1f;

			// TODO: now it <IMG> <amount>. need continue to: <IMG> <amount> OF <total> X<score> = <total_treasure_score>
		}

		break;

	case End:
		playNextLevel();
		break;

	default:
		// do nothing
		break;
	}
}
void LevelEndEngine::OnKeyUp(int key) { _state += 1; }
void LevelEndEngine::OnMouseButtonUp(MouseButtons btn) { _state += 1; }
void LevelEndEngine::playNextLevel()
{
	if (_lvlNum == 14) // last level
		changeEngine<CreditsEngine>();
	else
		changeEngine<LevelLoadingEngine>(_lvlNum + 1);
}
