#pragma once

#include "GameEngine/GameEngineFramework.hpp"


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

	struct Settings
	{
		bool details;    // 0-low, 1-high
		bool frontLayer; // 0-off, 1-on
		int8_t area;     // 0-9
		bool movies;     // 0-stretched, 1-interlaced
		int8_t soundVolume; // 0-9
		bool voice;         // 0-off, 1-on
		bool ambient;       // 0-off, 1-on
		int8_t musicVolume; // 0-9
	};
#pragma pack(pop)

	~SavedDataManager();

	void saveGame(const GameData& data);
	bool canLoadGame(int level, int savePoint);
	GameData loadGame(int level, int savePoint);


	static SavedDataManager instance;

	Settings settings;

private:
	SavedDataManager();
};
