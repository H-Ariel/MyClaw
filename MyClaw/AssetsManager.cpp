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

static inline bool isEnemyAniations(string dirPath)
{
	return contains(dirPath, "ANIS/") && (
		endsWith(dirPath, "OFFICER") ||
		endsWith(dirPath, "SOLDIER") ||
		endsWith(dirPath, "ROBBERTHIEF") ||
		endsWith(dirPath, "CUTTHROAT") ||
		endsWith(dirPath, "TOWNGUARD1") ||
		endsWith(dirPath, "TOWNGUARD2") ||
		endsWith(dirPath, "BEARSAILOR") ||
		endsWith(dirPath, "REDTAILPIRATE")
		);
	// TODO: add all of them
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

shared_ptr<UIBaseImage> AssetsManager::loadImage(string path)
{
	return _imagesManager->loadImage(path);
}
shared_ptr<Animation> AssetsManager::loadAnimation(string aniPath, string imageSetPath)
{
	return _animationsManager->loadAnimation(aniPath, imageSetPath);
}
shared_ptr<Animation> AssetsManager::loadCopyAnimation(string aniPath, string imageSetPath)
{
	return _animationsManager->loadAnimation(aniPath, imageSetPath, false);
}
shared_ptr<Animation> AssetsManager::createAnimationFromDirectory(string dirPath, uint32_t duration, bool reversedOrder)
{
	return _animationsManager->createAnimationFromDirectory(dirPath, duration, reversedOrder);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromDirectory(string dirPath, uint32_t duration, bool reversedOrder)
{
	return _animationsManager->createAnimationFromDirectory(dirPath, duration, reversedOrder)->getCopy();
}
shared_ptr<Animation> AssetsManager::createAnimationFromFromPidImage(string pidPath)
{
	return _animationsManager->createAnimationFromFromPidImage(pidPath);
}
shared_ptr<Animation> AssetsManager::createCopyAnimationFromFromPidImage(string pidPath)
{
	return _animationsManager->createAnimationFromFromPidImage(pidPath)->getCopy();
}
map<string, shared_ptr<Animation>> AssetsManager::loadAnimationsFromDirectory(string dirPath, string imageSetPath)
{
	map<string, shared_ptr<Animation>> anis;

	const RezDirectory* dir = _rezArchive->getDirectory(dirPath);
	if (dir)
	{
		for (auto& i : dir->_files)
		{
			string path = i.second->getFullPath();
			if (endsWith(path, ".ANI"))
			{
				anis[i.second->name] = _animationsManager->loadAnimation(path, imageSetPath, false);
			}
		}
	}

	// TODO: move to AnimationsManager and use Animation::getCopy()
	if (isEnemyAniations(dirPath))
	{
		vector<Animation::FrameData*> imagesList;
		vector<string> anisNames;
		for (auto& i : anis)
		{
			if (startsWith(i.first, "STAND") || startsWith(i.first, "IDLE"))
			{
				anisNames.push_back(i.first);
				for (Animation::FrameData* f : i.second->getImagesList())
				{
					if (f->duration == 0)
						myMemCpy(f->duration, 500U);
					imagesList.push_back(f);
				}
			}
		}

		for (auto& i : anisNames)
		{
			anis.erase(i);
		}

		anis["IDLE"] = allocNewSharedPtr<Animation>(imagesList);
	}

	return anis;
}
shared_ptr<WapWorld> AssetsManager::loadWwdFile(string wwdPath)
{
	return allocNewSharedPtr<WapWorld>(_rezArchive->getFileBufferReader(wwdPath));
}
shared_ptr<WapWorld> AssetsManager::loadLevelWwdFile(int8_t levelNumber)
{
	shared_ptr<WapWorld> wwd = loadWwdFile("LEVEL" + to_string((int)levelNumber) + "/WORLDS/WORLD.WWD");

	PathManager::setLevelRoot(levelNumber);
	
	if (levelNumber == 5)
	{
		wwd->tilesDescription[509].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		wwd->tilesDescription[509].outsideAttrib = WwdTileDescription::TileAttribute_Clear;
	}

	return wwd;
}
shared_ptr<PidPalette> AssetsManager::loadPidPalette(string palPath)
{
	shared_ptr<PidPalette> pal = allocNewSharedPtr<PidPalette>(_rezArchive->getFileData(palPath));
	_imagesManager->setPalette(pal);
	return pal;
}
map<int32_t, shared_ptr<UIBaseImage>> AssetsManager::loadPlaneTilesImages(string planeImagesPath)
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

shared_ptr<MidiPlayer> AssetsManager::getMidiPlayer(string xmiFilePath)
{
	return allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData(xmiFilePath));
}

// if debug - no sound
#ifndef _DEBUG
void AssetsManager::playWavFile(string wavFilePath, bool infinite)
{
	thread(&AudioManager::playWavFile, _audioManager, wavFilePath, infinite).detach();
}
void AssetsManager::stopWavFile(string wavFilePath)
{
	thread(&AudioManager::stopWavFile, _audioManager, wavFilePath).detach();
}
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type, bool reset)
{
	thread(&AudioManager::setBackgroundMusic, _audioManager, type, reset).detach();
}
#else
void AssetsManager::playWavFile(string wavFilePath, bool infinite) {}
void AssetsManager::stopWavFile(string wavFilePath) {}
void AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType type, bool reset) {}
#endif
uint32_t AssetsManager::getWavFileDuration(string wavFilePath)
{
	return _audioManager->getWavFileDuration(wavFilePath);
}
void AssetsManager::setWavFileVolume(string wavFilePath, int32_t volume)
{
	_audioManager->setVolume(wavFilePath, volume);
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
	}
}
