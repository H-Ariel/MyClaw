#include "AssetsManager.h"
#include "Objects/Item.h"


// check if `str` is number as string
static bool isNumber(string str)
{
	for (const char& c : str)
		if (!isdigit(c))
			return false;
	return true;
}


RezArchive* AssetsManager::_rezArchive = nullptr;
ImagesManager* AssetsManager::_imagesManager = nullptr;
AnimationsManager* AssetsManager::_animationsManager = nullptr;
AudioManager* AssetsManager::_audioManager = nullptr;
bool AssetsManager::_runApp = false;


void AssetsManager::Initialize()
{
	_rezArchive = DBG_NEW RezArchive("CLAW.REZ");
	_imagesManager = DBG_NEW ImagesManager(_rezArchive);
	_animationsManager = DBG_NEW AnimationsManager(_rezArchive);
	_audioManager = DBG_NEW AudioManager(_rezArchive);
	_runApp = true;
	srand((unsigned int)time(nullptr));
}
void AssetsManager::Finalize()
{
	_runApp = false;
	delete _imagesManager;
	delete _animationsManager;
	delete _audioManager;
	delete _rezArchive;
}

shared_ptr<UIBaseImage> AssetsManager::loadImage(const string& path)
{
	return _imagesManager->loadImage(path);
}
shared_ptr<Animation> AssetsManager::loadAnimation(const string& aniPath, const string& imageSetPath)
{
	return _animationsManager->loadAnimation(aniPath, imageSetPath);
}
shared_ptr<Animation> AssetsManager::loadCopyAnimation(const string& aniPath, const string& imageSetPath)
{
	return _animationsManager->loadAnimation(aniPath, imageSetPath, false);
}
shared_ptr<Animation> AssetsManager::createAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder)
{
	return _animationsManager->createAnimationFromDirectory(dirPath, duration, reversedOrder);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder)
{
	return _animationsManager->createAnimationFromDirectory(dirPath, duration, reversedOrder)->getCopy();
}
shared_ptr<Animation> AssetsManager::createAnimationFromFromPidImage(const string& pidPath)
{
	return _animationsManager->createAnimationFromFromPidImage(pidPath);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromFromPidImage(const string& pidPath)
{
	return _animationsManager->createAnimationFromFromPidImage(pidPath)->getCopy();
}
map<string, shared_ptr<Animation>> AssetsManager::loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath)
{
	return _animationsManager->loadAnimationsFromDirectory(dirPath, imageSetPath);
}
shared_ptr<WapWorld> AssetsManager::loadWwdFile(const string& wwdPath)
{
	return allocNewSharedPtr<WapWorld>(_rezArchive->getFileBufferReader(wwdPath));
}
shared_ptr<WapWorld> AssetsManager::loadLevelWwdFile(int levelNumber)
{
	shared_ptr<WapWorld> wwd = loadWwdFile("LEVEL" + to_string((int)levelNumber) + "/WORLDS/WORLD.WWD");

	PathManager::setLevelRoot(levelNumber);
	
	if (levelNumber == 5)
	{
		wwd->tilesDescription[509].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		wwd->tilesDescription[509].outsideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 11)
	{
		wwd->tilesDescription[39].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		wwd->tilesDescription[39].outsideAttrib = WwdTileDescription::TileAttribute_Clear;
	}

	return wwd;
}
shared_ptr<PidPalette> AssetsManager::loadPidPalette(const string& palPath)
{
	shared_ptr<PidPalette> pal = allocNewSharedPtr<PidPalette>(_rezArchive->getFileData(palPath));
	_imagesManager->setPalette(pal);
	return pal;
}
map<int32_t, shared_ptr<UIBaseImage>> AssetsManager::loadPlaneTilesImages(const string& planeImagesPath)
{
	map<int32_t, shared_ptr<UIBaseImage>> images;

	const RezDirectory* dir = _rezArchive->getDirectory(planeImagesPath);
	if (dir)
	{
		for (auto& i : dir->_files)
		{
			// the files path format is "LEVEL<N>/TILES/<PLN>/<XXX>.PID"
			if (i.second->isPidFile() && isNumber(i.second->name))
			{
				images[stoi(i.second->name)] = loadImage(i.second->getFullPath());
			}
		}
	}

	return images;
}

shared_ptr<MidiPlayer> AssetsManager::getMidiPlayer(const string& xmiFilePath)
{
	return allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData(xmiFilePath));
}

// if debug - no sound
#ifndef _DEBUG
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int32_t volume, bool infinite)
{
	uint32_t id = -1;
	try
	{
		id = _audioManager->playWavFile(wavFilePath, infinite);
		_audioManager->setVolume(id, volume);
	}
	catch (Exception& ex)
	{
		cout << __FUNCTION__ " - Error for wavFilePath=\"" << wavFilePath << "\": " << ex.what() << endl;
	}
	return id;
}
void AssetsManager::stopWavFile(uint32_t wavFileId)
{
	_audioManager->stopWavFile(wavFileId);
}
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type, bool reset)
{
	thread(&AudioManager::setBackgroundMusic, _audioManager, type, reset).detach();
}
#else
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int32_t volume, bool infinite) { return -1; }
void AssetsManager::stopWavFile(uint32_t wavFileId) {}
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type, bool reset) {}
#endif
uint32_t AssetsManager::getWavFileDuration(uint32_t wavFileId)
{
	return _audioManager->getWavFileDuration(wavFileId);
}

void AssetsManager::callLogics(uint32_t elapsedTime)
{
	_animationsManager->callAnimationsLogic(elapsedTime);
	_audioManager->checkForRestart();
}

void AssetsManager::clearLevelAssets(int lvl)
{
	if (_runApp)
	{
		const string prefix = "LEVEL" + to_string(lvl);
		_imagesManager->clearLevelImages(prefix);
		_animationsManager->clearLevelAnimations(prefix);
		_audioManager->clearLevelSounds(prefix);
		Item::resetItemsPaths();
		PathManager::resetPaths();
	}
}
