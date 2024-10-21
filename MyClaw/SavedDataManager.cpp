#include "SavedDataManager.h"


/*
File format:
 + SettingsData
 + GameData ( 14 levels * 3 save-points )
*/
#define SAVE_FILE_NAME "Claw.dat"


SavedDataManager SavedDataManager::instance;


static inline bool doesFileExists()
{
	return ifstream(SAVE_FILE_NAME, ios::binary).is_open();
}
static inline int getOffset(int level, int savePoint)
{
	int offset = (level - 1) * 3 + savePoint;
	return offset * sizeof(SavedDataManager::GameData) + sizeof(SavedDataManager::Settings);
}


SavedDataManager::SavedDataManager()
{
	if (!doesFileExists()) // initialize the file with default values
	{
		ofstream file(SAVE_FILE_NAME, ios::binary);

		Settings settings = {};
		settings.details = true;
		settings.frontLayer = true;
		settings.area = 9;
		settings.movies = false;
		settings.soundVolume = 9;
		settings.voice = true;
		settings.ambient = true;
		settings.musicVolume = 9;
		file.write((char*)&settings, sizeof(Settings));

		GameData data = {};
		const int n = 14 * 3; // 14 levels, 3 save points per level
		for (int i = 0; i < n; i++)
			file.write((char*)&data, sizeof(GameData));

		file.close();

		// create data for start of level 1:
		data.level = 1;
		data.savePoint = SavePoints::Start;
		data.lives = 6;
		data.health = 100;
		data.score = 0;
		data.pistolAmount = 10;
		data.magicAmount = 5;
		data.dynamiteAmount = 3;
		saveGame(data);
	}

	// load settings
	ifstream(SAVE_FILE_NAME, ios::binary)
		.read((char*)&settings, sizeof(Settings));
}
SavedDataManager::~SavedDataManager()
{
	// save settings
	fstream(SAVE_FILE_NAME, ios::binary | ios::in | ios::app) 
		.seekp(0).write((char*)&settings, sizeof(Settings));
}

void SavedDataManager::saveGame(const GameData& data)
{
	if (data.level < 1 || 14 < data.level) return;

	fstream(SAVE_FILE_NAME, ios::binary | ios::in | ios::out)
		.seekp(getOffset(data.level, data.savePoint))
		.write((char*)&data, sizeof(GameData));
}
bool SavedDataManager::canLoadGame(int level, int savePoint)
{
	if (level < 1 || 14 < level) return false;
	if (!doesFileExists()) return false;

	GameData data = loadGame(level, savePoint);
	return (data.level == level && data.savePoint == savePoint); // make sure the data is valid
}
SavedDataManager::GameData SavedDataManager::loadGame(int level, int savePoint)
{
	GameData data = {};
	ifstream(SAVE_FILE_NAME, ios::binary)
		.seekg(getOffset(level, savePoint))
		.read((char*)&data, sizeof(GameData));
	return data;
}
