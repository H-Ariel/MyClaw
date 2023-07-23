#include "AssetsManager.h"
#include "MidiPlayer.h"
#include "Objects/Item.h"


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
	_runApp = true;
	srand((unsigned int)time(nullptr));
}
AssetsManager::~AssetsManager()
{
	_runApp = false;
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
	if (instance != nullptr)
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
shared_ptr<Animation> AssetsManager::createAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder)
{
	return instance->_animationsManager->createAnimationFromDirectory(dirPath, duration, reversedOrder);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder)
{
	return instance->_animationsManager->createAnimationFromDirectory(dirPath, duration, reversedOrder)->getCopy();
}
shared_ptr<Animation> AssetsManager::createAnimationFromFromPidImage(const string& pidPath)
{
	return instance->_animationsManager->createAnimationFromFromPidImage(pidPath);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromFromPidImage(const string& pidPath)
{
	return instance->_animationsManager->createAnimationFromFromPidImage(pidPath)->getCopy();
}
map<string, shared_ptr<Animation>> AssetsManager::loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath)
{
	return instance->_animationsManager->loadAnimationsFromDirectory(dirPath, imageSetPath);
}
shared_ptr<WapWorld> AssetsManager::loadWwdFile(const string& wwdPath)
{
	return allocNewSharedPtr<WapWorld>(instance->_rezArchive->getFileBufferReader(wwdPath));
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
		string name;
		for (auto& i : dir->_files)
		{
			name = i.second->name.substr(0, i.second->name.length() - 4); // remove ".PID"
			// the files path format is "LEVEL<N>/TILES/<PLN>/<XXX>.PID"
			if (i.second->isPidFile() && isNumber(name))
			{
				images[stoi(name)] = loadImage(i.second->getFullPath());
			}
		}
	}

	return images;
}

shared_ptr<MidiPlayer> AssetsManager::getMidiPlayer(const string& xmiFilePath)
{
	return allocNewSharedPtr<MidiPlayer>(instance->_rezArchive->getFileData(xmiFilePath));
}

// if debug - no sound
#ifndef _DEBUG
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int32_t volume, bool infinite)
{
	uint32_t id = -1;
	try
	{
		id = instance->_audioManager->playWavFile(wavFilePath, infinite);
		instance->_audioManager->setVolume(id, volume);
	}
	catch (const Exception& ex)
	{
		cout << __FUNCTION__ " - Error for wavFilePath=\"" << wavFilePath << "\": " << ex.what() << endl;
	}
	return id;
}
void AssetsManager::stopWavFile(uint32_t wavFileId)
{
	instance->_audioManager->stopWavFile(wavFileId);
}
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type, bool reset)
{
	thread(&AudioManager::setBackgroundMusic, instance->_audioManager, type, reset).detach();
}
#else
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int32_t volume, bool infinite) { return -1; }
void AssetsManager::stopWavFile(uint32_t wavFileId) {}
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type, bool reset) {}
#endif
uint32_t AssetsManager::getWavFileDuration(uint32_t wavFileId)
{
	return instance->_audioManager->getWavFileDuration(wavFileId);
}

void AssetsManager::callLogics(uint32_t elapsedTime)
{
	instance->_animationsManager->callAnimationsLogic(elapsedTime);
	instance->_audioManager->checkForRestart();
}

void AssetsManager::clearLevelAssets(int lvl)
{
	if (instance && instance->_runApp)
	{
		const string prefix = "/LEVEL" + to_string(lvl);
		instance->_imagesManager->clearLevelImages(prefix);
		instance->_animationsManager->clearLevelAnimations(prefix);
		instance->_audioManager->clearLevelSounds();
		Item::resetItemsPaths();
		PathManager::resetPaths();
	}
}
