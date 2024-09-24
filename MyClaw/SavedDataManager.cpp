#include "SavedDataManager.h"


/*
File format:
 + Game Settings
  - 1 byte: details
  - 1 byte: front layer
  - 1 byte: movies
 + Game Data
  - 4 bytes: level
  - 4 bytes: save point
  - all the data of the save point
  * repeat for all levels (14) and save points (3)
*/
#define SAVE_FILE_NAME "Claw.dat"


static inline bool doesFileExists()
{
	return ifstream(SAVE_FILE_NAME, ios::binary).is_open();
}
static inline int getOffset(int level, int savePoint)
{
	int offset = (level - 1) * 3 + savePoint;
	return offset * sizeof(SavedDataManager::GameData) + sizeof(SavedDataManager::SettingsData);
}

void SavedDataManager::saveSettings(const SettingsData& data)
{
	// check if the file exists:
	if (!doesFileExists())
		initFile();

	ofstream(SAVE_FILE_NAME, ios::binary).write((char*)&data, sizeof(SettingsData));
}
SavedDataManager::SettingsData SavedDataManager::loadSettings()
{
	if (!doesFileExists())
		initFile(); // to load default settings

	SettingsData data = {};
	ifstream(SAVE_FILE_NAME, ios::binary).read((char*)&data, sizeof(SettingsData));
	return data;
}

void SavedDataManager::saveGame(const GameData& data)
{
	// check if the file exists:
	if (!doesFileExists())
		initFile();

	if (data.level < 1 || 14 < data.level) return;

	ofstream(SAVE_FILE_NAME, ios::binary)
		.seekp(getOffset(data.level, data.savePoint))
		.write((char*)&data, sizeof(GameData));
}
bool SavedDataManager::canLoadGame(int level, int savePoint)
{
	if (level < 1 || 14 < level) return false;
	if (!doesFileExists()) return false;

	GameData data = {};
	ifstream(SAVE_FILE_NAME, ios::binary)
		.seekg(getOffset(level, savePoint))
		.read((char*)&data, sizeof(GameData));

	return (data.level == level && data.savePoint == savePoint); // make sure the data is valid
}
SavedDataManager::GameData SavedDataManager::loadGame(int level, int savePoint)
{
	ifstream file(SAVE_FILE_NAME, ios::binary);

	GameData data = {};
	file.seekg(getOffset(level, savePoint));
	file.read((char*)&data, sizeof(GameData));

	return data;
}

void SavedDataManager::initFile()
{
	ofstream file(SAVE_FILE_NAME, ios::binary);
	GameData data = {};
	int n = 14 * 3; // 14 levels, 3 save points per level
	for (int i = 0; i < n; i++)
		file.write((char*)&data, sizeof(GameData));
	file.close();

	SettingsData settings = {};
	settings.details = true;
	settings.frontLayer = true;
	settings.movies = false;
	saveSettings(settings);

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
