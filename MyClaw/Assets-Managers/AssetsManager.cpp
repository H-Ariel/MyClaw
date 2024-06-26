#include "AssetsManager.h"
#include "GameEngine/AudioManager.h"
#include "GameEngine/WindowManager.h"
#include "RezParser/PidFile.h"
#include "RezParser/PcxFile.h"
#include "RezParser/XmiFile.h"
#include "RezParser/WwdFile.h"


// check if `str` is number as string
static bool isNumber(const string& str)
{
	for (const char& c : str)
		if (!isdigit(c))
			return false;
	return true;
}

static void fixPidOffset(const string& pidPath, int32_t& offsetX, int32_t& offsetY)
{
	// TODO: hack - edit the files?
	// NOTE: number-filenames are according to `fixFileName` at `RezArchive.cpp`


	// HUD pistol
	if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/PISTOL/001.PID")
	{
		offsetY += 2;
	}
	else if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/PISTOL/004.PID")
	{
		offsetY += 1;
	}
	// HUD magic
	else if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/MAGIC/001.PID")
	{
		offsetY += 2;
	}
	else if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/MAGIC/004.PID")
	{
		offsetY -= 1;
	}
	// HUD dynamite
	else if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/DYNAMITE/001.PID")
	{
		offsetY += 2;
	}
	// MagicClaw
	else if (pidPath == "CLAW/IMAGES/165.PID")
	{
		offsetX += 15;
	}
	else if (pidPath == "CLAW/IMAGES/166.PID")
	{
		offsetX += -5;
	}
	else if (pidPath == "CLAW/IMAGES/167.PID")
	{
		offsetX += 10;
	}
	// Claw swipe
	else if (pidPath == "CLAW/IMAGES/024.PID" || pidPath == "CLAW/IMAGES/025.PID" ||
		pidPath == "CLAW/IMAGES/026.PID" || pidPath == "CLAW/IMAGES/027.PID")
	{
		offsetX += 22;
	}
	// Cursor
	else if (pidPath == "GAME/IMAGES/CURSOR/004.PID")
	{
		offsetX += 2;
	}
	// Tower Cannon
	else if (pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/002.PID" ||
		pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/004.PID" ||
		pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/002.PID"
		|| pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/004.PID"
		|| pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/005.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL7/IMAGES/SANDHOLE/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL8/IMAGES/CANNONSWITCH/002.PID")
	{
		offsetX -= 2;
		offsetY -= 1;
	}
	// shooters from level 14
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/002.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/003.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/004.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/007.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/002.PID")
	{
		offsetX += 4;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/003.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/004.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/005.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/006.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/007.PID")
	{
		offsetX += 5;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/008.PID")
	{
		offsetX += 3;
	}
}



AssetsManager* AssetsManager::instance = nullptr;
const uint32_t AssetsManager::INVALID_AUDIOPLAYER_ID = AudioManager::INVALID_ID;


AssetsManager::AssetsManager()
	: _rezArchive("CLAW.REZ")
{
	_animationsManager = DBG_NEW AnimationsManager(&_rezArchive);
	_lastType = AssetsManager::BackgroundMusicType::None;
	srand((unsigned int)time(nullptr));

	// load background musics that used all over the game
	MidiFile boss(_rezArchive.getFile("LEVEL2/MUSIC/BOSS.XMI")->getFileData());
	// every level with boss contains the same file, so we can use LEVEL2/BOSS.XMI for all levels
	AudioManager::addMidiPlayer("BOSS", boss.data);
	MidiFile powerup(_rezArchive.getFile("GAME/MUSIC/POWERUP.XMI")->getFileData());
	AudioManager::addMidiPlayer("POWERUP", powerup.data);
	MidiFile credits(_rezArchive.getFile("STATES/CREDITS/MUSIC/PLAY.XMI")->getFileData());
	AudioManager::addMidiPlayer("CREDITS", credits.data);
}
AssetsManager::~AssetsManager()
{
	delete _animationsManager;
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
	shared_ptr<UIBaseImage> img;

	if (WindowManager::hasImage(path))
	{
		img = WindowManager::getImage(path);
	}
	else
	{
		try
		{
			if (endsWith(path, ".PID"))
			{
				WapPid pid(instance->_rezArchive.getFile(path)->getFileData(), &instance->_palette);

				fixPidOffset(path, pid.offsetX, pid.offsetY);

				img = WindowManager::createImage(path, pid.colors.data(),
					pid.width, pid.height, (float)pid.offsetX, (float)pid.offsetY);
			}
			else if (endsWith(path, ".PCX"))
			{
				PcxFile pcx(instance->_rezArchive.getFile(path)->getBufferReader());

				img = WindowManager::createImage(path, pcx.colors.data(),
					pcx.width, pcx.height, 0, 0);

				// pcx files saves their palette and the palette is used for images at score screen
				instance->_palette = pcx.palette;
			}
			else throw Exception("empty imgae");
		}
		catch (const Exception& ex)
		{
			img = make_shared<UIBaseImage>(nullptr); // empty image
			DBG_PRINT("WARNING: a blank image has been inserted. image path: \"%s\"\n", path.c_str());
		}
	}

	return img;
}
map<int, shared_ptr<UIBaseImage>> AssetsManager::loadPlaneTilesImages(const string& planeImagesPath)
{
	map<int, shared_ptr<UIBaseImage>> images;

	const RezDirectory* dir = instance->_rezArchive.getDirectory(planeImagesPath);
	if (dir)
	{
		string newname;
		for (auto& [filename, file] : dir->rezFiles)
		{
			newname = filename.substr(0, filename.length() - 4); // remove ".PID"
			// the files path format is "LEVEL<N>/TILES/<PLN>/<XXX>.PID"
			if (!strcmp(file->extension, "PID") && isNumber(newname))
				images[stoi(newname)] = loadImage(file->getFullPath());
		}
	}

	return images;
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

shared_ptr<WapWwd> AssetsManager::loadLevel(int levelNumber)
{
	char path[25];
	sprintf(path, "LEVEL%d/WORLDS/WORLD.WWD", levelNumber);
	shared_ptr<WapWwd> wwd = make_shared<WapWwd>(instance->_rezArchive.getFile(path)->getBufferReader(), levelNumber);

	// initialize level palette
	loadPidPalette(wwd->rezPalettePath);

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
		instance->_palette.colors[wwd->planes[0].fillColor].r / 255.f,
		instance->_palette.colors[wwd->planes[0].fillColor].g / 255.f,
		instance->_palette.colors[wwd->planes[0].fillColor].b / 255.f
	));

	// add all wav files to audio manager
	instance->loadWavDir("CLAW/SOUNDS");
	instance->loadWavDir("GAME/SOUNDS");
	sprintf(path, "LEVEL%d/SOUNDS", wwd->levelNumber);
	instance->loadWavDir(path);

	// add midi background music
	MidiFile level(instance->_rezArchive.getFile(PathManager::getBackgroundMusicFilePath("LEVEL_PLAY"))->getFileData());
	AudioManager::addMidiPlayer("LEVEL", level.data);

	return wwd;
}
void AssetsManager::loadPidPalette(const string& palPath)
{
	instance->_palette = WapPal(instance->_rezArchive.getFile(palPath)->getFileData());
}
string AssetsManager::getCreditsText()
{
	const RezFile* file = instance->_rezArchive.getFile("STATES/CREDITS/CREDITS.TXT");
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
		shared_ptr<BufferReader> reader = instance->_rezArchive.getFile(wavFilePath)->getBufferReader();
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
	AudioManager::remove(wavId);
}
#else
uint32_t AssetsManager::playWavFile(const string& wavFilePath, int volume, bool infinite)
{
	return INVALID_AUDIOPLAYER_ID;
}
void AssetsManager::stopWavFile(uint32_t wavId) {}
#endif
uint32_t AssetsManager::getWavFileDuration(const string& wavFileKey)
{
	return AudioManager::getDuration(wavFileKey);
}

#ifndef _DEBUG // if debug - no background music
void AssetsManager::startBackgroundMusic(BackgroundMusicType type)
{
	if (instance->_lastType == type)
		return;

	stopBackgroundMusic();
	instance->_lastType = type;

	uint32_t id = 0;
	switch (type)
	{
	case BackgroundMusicType::Level: id = AudioManager::playMidi("LEVEL", true); break;
	case BackgroundMusicType::Powerup: id = AudioManager::playMidi("POWERUP", true); break;
	case BackgroundMusicType::Boss: id = AudioManager::playMidi("BOSS", true); break;
	case BackgroundMusicType::Credits: id = AudioManager::playMidi("CREDITS", true); break;
	}
	if (id != INVALID_AUDIOPLAYER_ID)
		instance->bgMusics[type] = id;
}
void AssetsManager::stopBackgroundMusic()
{
	if (instance->_lastType == BackgroundMusicType::None)
		return;

	uint32_t id = instance->bgMusics[instance->_lastType];
	if (id != AudioManager::INVALID_ID)
		AudioManager::stop(id);

	instance->_lastType = BackgroundMusicType::None; // no background music
}
#else
void AssetsManager::startBackgroundMusic(BackgroundMusicType type) {}
void AssetsManager::stopBackgroundMusic() {}
#endif

void AssetsManager::callLogics(uint32_t elapsedTime)
{
	instance->_animationsManager->callAnimationsLogic(elapsedTime);
}

void AssetsManager::clearLevelAssets(int lvl)
{
	if (instance)
	{
		const string prefix = "LEVEL";// +to_string(lvl);
		auto removeByPrefix = [&prefix](const string& key) { return startsWith(key, prefix); };

		WindowManager::clearImages(removeByPrefix);
		instance->_animationsManager->clearAnimations(removeByPrefix);

		stopBackgroundMusic();
		AudioManager::remove(removeByPrefix);
		AudioManager::remove(instance->bgMusics[BackgroundMusicType::Level]);
		instance->bgMusics[BackgroundMusicType::Level] = INVALID_AUDIOPLAYER_ID;
		PathManager::resetPaths();
	}
}

void AssetsManager::loadWavDir(const string& path)
{
	if (const RezDirectory* dir = _rezArchive.getDirectory(path))
	{
		for (auto& [dirname, dir] : dir->rezDirectories)
		{
			loadWavDir(dir->getFullPath());
		}
		for (auto& [filename, file] : dir->rezFiles)
		{
			AudioManager::addWavPlayer(file->getFullPath(), file->getBufferReader());
		}
	}
}
