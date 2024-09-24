#pragma once

#include "Framework/Framework.h"


class SavedDataManager
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
		int32_t savePoint; // SavePoints
		// player data:
		int32_t lives;
		int32_t health;
		int32_t score;
		int32_t pistolAmount;
		int32_t magicAmount;
		int32_t dynamiteAmount;
	};

	struct SettingsData
	{
		bool details;    // 0-low, 1-high
		bool frontLayer; // 0-off, 1-on
		bool movies;     // 0-stretched, 1-interlaced
	};
#pragma pack(pop)

	static void saveSettings(const SettingsData& data);
	static SettingsData loadSettings();

	static void saveGame(const GameData& data);
	static bool canLoadGame(int level, int savePoint);
	static GameData loadGame(int level, int savePoint);

private:
	static void initFile(); // initialize the file with empty data
};
