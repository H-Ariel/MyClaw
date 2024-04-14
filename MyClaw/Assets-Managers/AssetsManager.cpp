#include "AssetsManager.h"
#include "GameEngine/AudioManager.h"
#include "GameEngine/WindowManager.h"


// check if `str` is number as string
static bool isNumber(string str)
{
	for (const char& c : str)
		if (!isdigit(c))
			return false;
	return true;
}

// load all wav files from directory
static void loadWavDir(RezArchive* rez, const string& path)
{
	const RezDirectory* dir = rez->getDirectory(path);
	if (dir)
	{
		for (auto& [dirname, dir] : dir->rezDirectories)
		{
			loadWavDir(rez, dir->getFullPath());
		}
		for (auto& [filename, file] : dir->rezFiles)
		{
			AudioManager::addWavPlayer(file->getFullPath(), file->getBufferReader());
		}
	}
}

// initialize more assets. TODO: move to other place
static void LevelInit(shared_ptr<WapWwd> wwd, RezArchive* rez)
{
	char path[25];

	// initialize level palette
	shared_ptr<WapPal> pal = AssetsManager::loadPidPalette(wwd->rezPalettePath);

	// initialize level root paths
	string imageSet[4], prefix[4];
	for (int i = 0; i < 4; i++)
	{
		imageSet[i] = wwd->imageSet[i];
		prefix[i] = wwd->prefix[i];
	}
	PathManager::setRoots(prefix, imageSet);

	// set background color
	WindowManager::setBackgroundColor(ColorF(
		pal->colors[wwd->planes[0].fillColor].r / 255.f,
		pal->colors[wwd->planes[0].fillColor].g / 255.f,
		pal->colors[wwd->planes[0].fillColor].b / 255.f
	));

	// add all wav files to audio manager
	loadWavDir(rez, "CLAW/SOUNDS");
	loadWavDir(rez, "GAME/SOUNDS");
	sprintf(path, "LEVEL%d/SOUNDS", wwd->levelNumber);
	loadWavDir(rez, path);

	// add midi background music
	MidiFile level(rez->getFile(PathManager::getBackgroundMusicFilePath("LEVEL_PLAY"))->getFileData());
	AudioManager::addMidiPlayer("level", level.data);

	MidiFile boss(rez->getFile("LEVEL2/MUSIC/BOSS.XMI")->getFileData());
	// every level with boss contains the same file, so we can use LEVEL2/BOSS.XMI for all levels
	AudioManager::addMidiPlayer("boss", boss.data);

	MidiFile powerup(rez->getFile("GAME/MUSIC/POWERUP.XMI")->getFileData());
	AudioManager::addMidiPlayer("powerup", powerup.data);

	MidiFile credits(rez->getFile("STATES/CREDITS/MUSIC/PLAY.XMI")->getFileData());
	AudioManager::addMidiPlayer("credits", powerup.data);
}



AssetsManager* AssetsManager::instance = nullptr;
const uint32_t AssetsManager::INVALID_AUDIOPLAYER_ID = AudioManager::INVALID_ID;


AssetsManager::AssetsManager()
{
	_rezArchive = DBG_NEW RezArchive();
	_imagesManager = DBG_NEW ImagesManager(_rezArchive);
	_animationsManager = DBG_NEW AnimationsManager(_rezArchive);
	_lastType = AssetsManager::BackgroundMusicType::None;
	srand((unsigned int)time(nullptr));
}
AssetsManager::~AssetsManager()
{
	SafeDelete(_imagesManager);
	SafeDelete(_animationsManager);
	SafeDelete(_rezArchive);
}

void AssetsManager::Initialize()
{
	if (instance == nullptr)
		instance = DBG_NEW AssetsManager();
}
void AssetsManager::Finalize()
{
	if (instance)
	{
		delete instance;
		instance = nullptr;
	}
}

shared_ptr<UIBaseImage> AssetsManager::loadImage(const string& path)
{
	return instance->_imagesManager->loadImage(path);
}
shared_ptr<UIAnimation> AssetsManager::loadAnimation(const string& aniPath, const string& imageSetPath)
{
	return instance->_animationsManager->loadAnimation(aniPath, imageSetPath);
}
shared_ptr<UIAnimation> AssetsManager::loadCopyAnimation(const string& aniPath, const string& imageSetPath)
{
	return instance->_animationsManager->loadAnimation(aniPath, imageSetPath, false);
}
shared_ptr<UIAnimation> AssetsManager::createAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration)
{
	return instance->_animationsManager->createAnimationFromDirectory(dirPath, reversedOrder, duration);
}
shared_ptr<UIAnimation> AssetsManager::createCopyAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration)
{
	return createAnimationFromDirectory(dirPath, reversedOrder, duration)->getCopy();
}
shared_ptr<UIAnimation> AssetsManager::createAnimationFromPidImage(const string& pidPath)
{
	return instance->_animationsManager->createAnimationFromPidImage(pidPath);
}
shared_ptr<UIAnimation> AssetsManager::createCopyAnimationFromPidImage(const string& pidPath)
{
	return createAnimationFromPidImage(pidPath)->getCopy();
}
map<string, shared_ptr<UIAnimation>> AssetsManager::loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath)
{
	return instance->_animationsManager->loadAnimationsFromDirectory(dirPath, imageSetPath);
}

shared_ptr<WapWwd> AssetsManager::loadLevelWwdFile(int levelNumber)
{
	char path[25];
	sprintf(path, "LEVEL%d/WORLDS/WORLD.WWD", levelNumber);
	shared_ptr<WapWwd> wwd = allocNewSharedPtr<WapWwd>(instance->_rezArchive->getFile(path)->getBufferReader(), levelNumber);
	LevelInit(wwd, instance->_rezArchive);
	return wwd;
}
shared_ptr<WapPal> AssetsManager::loadPidPalette(const string& palPath)
{
	shared_ptr<WapPal> pal(DBG_NEW WapPal(instance->_rezArchive->getFile(palPath)->getFileData()));
	instance->_imagesManager->setPalette(pal);
	return pal;
}
map<int32_t, shared_ptr<UIBaseImage>> AssetsManager::loadPlaneTilesImages(const string& planeImagesPath)
{
	map<int32_t, shared_ptr<UIBaseImage>> images;

	const RezDirectory* dir = instance->_rezArchive->getDirectory(planeImagesPath);
	if (dir)
	{
		string newname;
		for (auto& [filename, file] : dir->rezFiles)
		{
			newname = filename.substr(0, filename.length() - 4); // remove ".PID"
			// the files path format is "LEVEL<N>/TILES/<PLN>/<XXX>.PID"
			if (!strcmp(file->extension, "PID") && isNumber(newname))
			{
				images[stoi(newname)] = loadImage(file->getFullPath());
			}
		}
	}

	return images;
}

string AssetsManager::getCreditsText()
{
	const RezFile* file = instance->_rezArchive->getFile("STATES/CREDITS/CREDITS.TXT");
	vector<uint8_t> data = file->getFileData();
	return string((char*)data.data(), data.size());
}

#ifndef _DEBUG // if debug - no sounds
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int volume, bool infinite)
{
	uint32_t id = AudioManager::playWav(wavFilePath, infinite);

	if (id == AudioManager::INVALID_ID)
	{
		// load WAV to audio manager
		shared_ptr<BufferReader> reader = instance->_rezArchive->getFile(wavFilePath)->getBufferReader();
		AudioManager::addWavPlayer(wavFilePath, reader);

		// play WAV
		id = AudioManager::playWav(wavFilePath, infinite);
	}

	AudioManager::setVolume(id, volume);
	return id;
}
void AssetsManager::stopWavFile(uint32_t wavId)
{
	AudioManager::stop(wavId);
}
#else
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int volume, bool infinite)
{
	return INVALID_AUDIOPLAYER_ID;
}
void AssetsManager::stopWavFile(uint32_t wavId) {}
#endif

#ifndef _DEBUG // if debug - no background music
void AssetsManager::setBackgroundMusic(BackgroundMusicType type)
{
	if (instance->_lastType == type)
		return;

	stopBackgroundMusic();
	instance->_lastType = type;

	auto it = instance->bgMusics.find(type);
	if (it != instance->bgMusics.end())
	{
		AudioManager::continuePlay(it->second);
	}
	else
	{
		uint32_t id = UINT32_MAX;
		switch (type)
		{
		case BackgroundMusicType::Level: id = AudioManager::playMidi("level", true); break;
		case BackgroundMusicType::Powerup: id = AudioManager::playMidi("powerup", true); break;
		case BackgroundMusicType::Boss: id = AudioManager::playMidi("boss", true); break;
		case BackgroundMusicType::Credits: id = AudioManager::playMidi("credits", true); break;
		}
		if (id != AudioManager::INVALID_ID)
			instance->bgMusics[type] = id;
	}
}
void AssetsManager::stopBackgroundMusic()
{
	if (instance->_lastType == BackgroundMusicType::None)
		return;

	uint32_t id = instance->bgMusics[instance->_lastType];
	if (id != AudioManager::INVALID_ID)
		AudioManager::stop(id);
}
#else
void AssetsManager::setBackgroundMusic(BackgroundMusicType type) {}
void AssetsManager::stopBackgroundMusic() {}
#endif
uint32_t AssetsManager::getWavFileDuration(const string& wavFileKey)
{
	return AudioManager::getDuration(wavFileKey);
}

// calls logics for all animations and sounds
void AssetsManager::callLogics(uint32_t elapsedTime)
{
	instance->_animationsManager->callAnimationsLogic(elapsedTime);
	AudioManager::checkForRestart();
}

void AssetsManager::clearLevelAssets(int lvl)
{
	if (instance)
	{
		const string prefix = "LEVEL" + to_string(lvl);
		instance->_imagesManager->clearLevelImages(prefix);
		instance->_animationsManager->clearLevelAnimations(prefix);
		stopBackgroundMusic();
		AudioManager::remove([&prefix](const string& key) { return key.find(prefix) != string::npos; });
		AudioManager::remove(instance->bgMusics[BackgroundMusicType::Level]);
		instance->bgMusics.erase(BackgroundMusicType::Level);
		instance->_lastType = BackgroundMusicType::None;
		PathManager::resetPaths();
	}
}
