#include "SavedGameManager.h"


/*
File format:
  - 4 bytes: level
  - 4 bytes: save point
  - all the data of the save point
  * repeat for all levels
*/
#define SAVE_FILE_NAME "CLAW_SAVE.DAT"


bool isFileExists(const char* fileName)
{
	ifstream file(fileName);
	return file.is_open();
}

int getOffset(int32_t level, SavedGameManager::SavePoints savePoint)
{
	return (level - 1) * 3 + (int)savePoint;
}


bool SavedGameManager::hasSavedGame()
{
	return isFileExists(SAVE_FILE_NAME);
}

void SavedGameManager::save(const GameData& data)
{
	// check if the file exists:

	if (!hasSavedGame())
	{
		// initialize the file with empty data:
		ofstream file(SAVE_FILE_NAME, ios::binary);
		GameData data = {};
		int n = 14 * 3; // 14 levels, 3 save points per level
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
		save(data);
	}

	if (data.level < 1 || 14 < data.level) return;

	// TODO: save but not in the order i want (it reset the file)
	ofstream file(SAVE_FILE_NAME, ios::binary);
	file.seekp(getOffset(data.level, data.savePoint) * sizeof(GameData));
	file.write((char*)&data, sizeof(GameData));

	// TDOD: write to screen that the game was saved
	MessageBox(nullptr, L"Game saved", L"Saved", MB_OK | MB_ICONINFORMATION);
}

bool SavedGameManager::canLoadGame(int32_t level, SavePoints savePoint)
{
	if (!hasSavedGame()) return false;
	if (level < 1 || 14 < level) return false;

	ifstream file(SAVE_FILE_NAME, ios::binary);

	GameData data = {};
	file.seekg(getOffset(level, savePoint) * sizeof(GameData));
	file.read((char*)&data, sizeof(GameData));

	return (data.level == level && data.savePoint == savePoint);
}

SavedGameManager::GameData SavedGameManager::load(int32_t level, SavePoints savePoint)
{
	if (!canLoadGame(level, savePoint)) throw Exception("cannot load game");

	ifstream file(SAVE_FILE_NAME, ios::binary);

	GameData data = {};
	file.seekg(getOffset(level, savePoint) * sizeof(GameData));
	file.read((char*)&data, sizeof(GameData));

	return data;
}
