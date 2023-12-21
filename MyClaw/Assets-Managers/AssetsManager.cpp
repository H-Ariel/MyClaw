#include "AssetsManager.h"
#include "MidiPlayer.h"
#include "../Objects/Item.h"


// check if `str` is number as string
static bool isNumber(string str)
{
	for (const char& c : str)
		if (!isdigit(c))
			return false;
	return true;
}


AssetsManager* AssetsManager::instance = nullptr;


AssetsManager::AssetsManager()
{
	_rezArchive = DBG_NEW RezArchive("CLAW.REZ");
	_imagesManager = DBG_NEW ImagesManager(_rezArchive);
	_animationsManager = DBG_NEW AnimationsManager(_rezArchive);
	_audioManager = DBG_NEW AudioManager(_rezArchive);
	srand((unsigned int)time(nullptr));
}
AssetsManager::~AssetsManager()
{
	SafeDelete(_imagesManager);
	SafeDelete(_animationsManager);
	SafeDelete(_audioManager);
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
shared_ptr<Animation> AssetsManager::loadAnimation(const string& aniPath, const string& imageSetPath)
{
	return instance->_animationsManager->loadAnimation(aniPath, imageSetPath);
}
shared_ptr<Animation> AssetsManager::loadCopyAnimation(const string& aniPath, const string& imageSetPath)
{
	return instance->_animationsManager->loadAnimation(aniPath, imageSetPath, false);
}
shared_ptr<Animation> AssetsManager::createAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration)
{
	return instance->_animationsManager->createAnimationFromDirectory(dirPath, reversedOrder, duration);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration)
{
	return createAnimationFromDirectory(dirPath, reversedOrder, duration)->getCopy();
}
shared_ptr<Animation> AssetsManager::createAnimationFromPidImage(const string& pidPath)
{
	return instance->_animationsManager->createAnimationFromPidImage(pidPath);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromPidImage(const string& pidPath)
{
	return createAnimationFromPidImage(pidPath)->getCopy();
}
map<string, shared_ptr<Animation>> AssetsManager::loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath)
{
	return instance->_animationsManager->loadAnimationsFromDirectory(dirPath, imageSetPath);
}
shared_ptr<WapWorld> AssetsManager::loadLevelWwdFile(int levelNumber)
{
	char path[25];
	sprintf(path, "LEVEL%d/WORLDS/WORLD.WWD", levelNumber);
	return allocNewSharedPtr<WapWorld>(instance->_rezArchive->getFileBufferReader(path), levelNumber);
}
shared_ptr<PidPalette> AssetsManager::loadPidPalette(const string& palPath)
{
	shared_ptr<PidPalette> pal(DBG_NEW PidPalette(instance->_rezArchive->getFileData(palPath)));
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
		for (auto& [filename, file] : dir->_files)
		{
			newname = filename.substr(0, filename.length() - 4); // remove ".PID"
			// the files path format is "LEVEL<N>/TILES/<PLN>/<XXX>.PID"
			if (file->isPidFile() && isNumber(newname))
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
	vector<uint8_t> data = file->getData();
	return string((char*)data.data(), data.size());
}

shared_ptr<MidiPlayer> AssetsManager::getMidiPlayer(const string& xmiFilePath)
{
	return allocNewSharedPtr<MidiPlayer>(instance->_rezArchive->getFileData(xmiFilePath));
}

#ifndef _DEBUG // if debug - no sounds
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int volume, bool infinite)
{
	uint32_t id = -1;
	try
	{
		id = instance->_audioManager->playWavFile(wavFilePath, infinite);
		instance->_audioManager->setVolume(id, volume);
	}
	catch (const Exception& ex)
	{
		DBG_PRINT(__FUNCTION__ " - Error for wavFilePath=\"%s\": %s\n", wavFilePath.c_str(), ex.what().c_str());
	}
	return id;
}
void AssetsManager::stopWavFile(uint32_t wavFileId)
{
	instance->_audioManager->stopWavFile(wavFileId);
}
#else
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int volume, bool infinite) { return -1; }
void AssetsManager::stopWavFile(uint32_t wavFileId) {}
#endif

#ifndef _DEBUG0 // if debug - no background music
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type)
{
	//thread(&AudioManager::setBackgroundMusic, instance->_audioManager, type).detach();
	instance->_audioManager->setBackgroundMusic(type);
}
void AssetsManager::stopBackgroundMusic()
{
	instance->_audioManager->stopBackgroundMusic();
}
#else
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type) {}
void AssetsManager::stopBackgroundMusic() {}
#endif
uint32_t AssetsManager::getWavFileDuration(uint32_t wavFileId)
{
	return instance->_audioManager->getWavFileDuration(wavFileId);
}

// calls logics for all animations and sounds
void AssetsManager::callLogics(uint32_t elapsedTime)
{
	instance->_animationsManager->callAnimationsLogic(elapsedTime);
	instance->_audioManager->checkForRestart();
}

void AssetsManager::clearLevelAssets(int lvl)
{
	if (instance)
	{
		const string prefix = "LEVEL" + to_string(lvl);
		instance->_imagesManager->clearLevelImages(prefix);
		instance->_animationsManager->clearLevelAnimations(prefix);
		instance->_audioManager->clearLevelSounds(prefix);
		Item::resetItemsPaths();
		PathManager::resetPaths();
	}
}
