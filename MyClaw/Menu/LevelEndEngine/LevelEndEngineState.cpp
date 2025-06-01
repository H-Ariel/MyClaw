#include "LevelEndEngineState.h"
#include "LevelEndEngineHelper.h"
#include "../MenuItem.h"
#include "../CreditsEngine.h"
#include "../LevelLoadingEngine.h"


// TODO: draw cool animation of map/gem before showing score


constexpr int NUM_OF_TREASURES = 9;

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


void StartState::Logic() {}
LevelEndEngineState* StartState::nextState() { return DBG_NEW DrawScoreState(_levelEndEngine); }

void DrawScoreState::Logic() {
	float x, y;
	int i, j;

	delete _levelEndEngine->_bgImg;
	_levelEndEngine->_elementsList.clear();
	_levelEndEngine->_elementsList.push_back(_levelEndEngine->_bgImg = DBG_NEW MenuBackgroundImage(getBGImgPath(_levelEndEngine->_lvlNum * 2)));

	y = -0.38f;

	// draw all treasures and their points
	for (i = 0; i < NUM_OF_TREASURES; i++)
	{
		x = -0.12f;

		// draw current treasures
		_levelEndEngine->_elementsList.push_back(DBG_NEW MenuItem(treasuresData[i].second, -0.25f, y, _levelEndEngine->_bgImg, _levelEndEngine));

		// draw collected treasures count (3 digits)
		int score = _levelEndEngine->_collectedTreasures[treasuresData[i].first];

		for (j = 0; j < 3; j++)
		{
			ScoreNumberPath[35] = (score % 10) + '0';
			_levelEndEngine->_elementsList.push_back(DBG_NEW MenuItem(ScoreNumberPath, x, y, _levelEndEngine->_bgImg, _levelEndEngine));
			x -= 0.02f;
			score /= 10;
		}

		y += 0.1f;

		// TODO: now it <IMG> <amount>. need continue to: <IMG> <amount> OF <total> X<score> = <total_treasure_score>
	}

	_levelEndEngine->nextState();
}
LevelEndEngineState* DrawScoreState::nextState() { return DBG_NEW WaitState(_levelEndEngine); }

void WaitState::Logic() {}
LevelEndEngineState* WaitState::nextState() { return DBG_NEW EndState(_levelEndEngine); }

void EndState::Logic()
{
	if (_levelEndEngine->_lvlNum == 14) // show credits after last level
		_levelEndEngine->changeEngine(DBG_NEW CreditsEngine());
	else // play the next level
		_levelEndEngine->changeEngine(DBG_NEW LevelLoadingEngine(_levelEndEngine->_lvlNum + 1));
}
LevelEndEngineState* EndState::nextState() { return nullptr; }
