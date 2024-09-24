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
		int8_t level;
		int8_t savePoint; // SavePoints
		// player data:
		int8_t lives;
		int8_t health;
		int32_t score;
		int8_t pistolAmount;
		int8_t magicAmount;
		int8_t dynamiteAmount;
	};

	struct SettingsData
	{
		bool details;    // 0-low, 1-high
		bool frontLayer; // 0-off, 1-on
		bool movies;     // 0-stretched, 1-interlaced
		int8_t soundVolume; // 0-100
		bool voice;         // 0-off, 1-on
		bool ambient;       // 0-off, 1-on
		int8_t musicVolume; // 0-100
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
