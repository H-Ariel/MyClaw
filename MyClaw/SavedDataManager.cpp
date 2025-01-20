#include "SavedDataManager.h"


/*
File format:
 + SettingsData
 + GameData ( 14 levels * 3 save-points )
*/

constexpr const char* SAVE_FILE_NAME = "Claw.dat";
constexpr int MAX_LEVEL = 14;
constexpr int CHECKPOINTS_COUNT = 3;


SavedDataManager::Settings SavedDataManager::settings;


template<typename T>
std::istream& operator>>(std::istream& stream, T& obj) {
	stream.read(reinterpret_cast<char*>(&obj), sizeof(T));
	return stream;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const T& obj) {
	stream.write(reinterpret_cast<const char*>(&obj), sizeof(T));
	return stream;
}


static inline bool doesFileExists()
{
	return ifstream(SAVE_FILE_NAME, ios::binary).is_open();
}
static inline int getOffset(int level, int savePoint)
{
	int offset = (level - 1) * CHECKPOINTS_COUNT + savePoint;
	return offset * sizeof(SavedDataManager::GameData) + sizeof(SavedDataManager::Settings);
}

void SavedDataManager::Initialize() {
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
		file << settings;

		GameData data = {};
		const int n = MAX_LEVEL * CHECKPOINTS_COUNT;
		for (int i = 0; i < n; i++)
			file << data;

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
	ifstream(SAVE_FILE_NAME, ios::binary) >> settings;
}
void SavedDataManager::Finalize() {
	// save settings
	fstream(SAVE_FILE_NAME, ios::binary | ios::in | ios::out)
		.seekp(0) << settings;
}

void SavedDataManager::saveGame(const GameData& data)
{
	if (data.level < 1 || MAX_LEVEL < data.level) return;

	fstream(SAVE_FILE_NAME, ios::binary | ios::in | ios::out)
		.seekp(getOffset(data.level, data.savePoint)) << data;
}
bool SavedDataManager::canLoadGame(int level, int savePoint)
{
	if (level < 1 || MAX_LEVEL < level) return false;
	if (!doesFileExists()) return false;

	GameData data = loadGame(level, savePoint);
	return (data.level == level && data.savePoint == savePoint); // make sure the data is valid
}
SavedDataManager::GameData SavedDataManager::loadGame(int level, int savePoint)
{
	GameData data;
	ifstream(SAVE_FILE_NAME, ios::binary)
		.seekg(getOffset(level, savePoint)) >> data;
	return data;
}
