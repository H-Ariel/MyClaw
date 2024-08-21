#pragma once

#include "Framework/Framework.h"


class SavedGameManager
{
public:
	enum SavePoints
	{
		Start,
		SuperCheckpoint1,
		SuperCheckpoint2
	};

#pragma pack(push, 1)
	struct GameData
	{
		// save point data:
		int32_t level;
		int32_t savePoint;
		// player data:
		int32_t lives;
		int32_t health;
		int32_t score;
		int32_t pistolAmount;
		int32_t magicAmount;
		int32_t dynamiteAmount;
	};
#pragma pack(pop)

	static bool hasSavedGame();
	static void save(const GameData& data);
	static bool canLoadGame(int level, int savePoint);
	static GameData load(int level, int savePoint); // if can't load, return empty data
};
