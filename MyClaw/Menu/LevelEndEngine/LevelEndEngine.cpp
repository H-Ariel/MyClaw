#include "LevelEndEngineState.h"
#include "LevelEngine/GlobalObjects.h"
#include "LevelEngine/Objects/Player.h"
#include "LevelEndEngineHelper.h"


LevelEndEngine::LevelEndEngine(int lvlNum, const map<Item::Type, uint32_t>& collectedTreasures)
	: ScreenEngine(getBGImgPath(lvlNum * 2 - 1)), _lvlNum(lvlNum), 
	_state(DBG_NEW StartState(this)), _nextState(nullptr)
{
	if (_lvlNum != 14)
	{
		// save as checkpoint:
		SavedDataManager::GameData data = GO::player->getGameData();
		data.level = _lvlNum + 1;
		data.savePoint = SavedDataManager::SavePoints::Start;
		SavedDataManager::saveGame(data);
	}

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

		default: throw Exception("invalid treasure type");
		}

		_collectedTreasures[tresType] += i.second;
	}
}
LevelEndEngine::~LevelEndEngine()
{
	delete _state;
	delete _nextState;
}
void LevelEndEngine::Logic(uint32_t elapsedTime)
{
	ScreenEngine::Logic(elapsedTime);

	if (!_state->logicDone) {
		_state->Logic();
		_state->logicDone = true;
	}

	if (_nextState) {
		delete _state;
		_state = _nextState;
		_nextState = nullptr;
	}
}
void LevelEndEngine::OnKeyUp(int key) { nextState(); }
void LevelEndEngine::OnMouseButtonUp(MouseButtons btn) { nextState(); }
inline void LevelEndEngine::nextState()
{
	_nextState = _state->nextState();
}
