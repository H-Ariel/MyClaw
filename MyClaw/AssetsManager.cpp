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
shared_ptr<WapWorld> AssetsManager::loadLevelWwdFile(int8_t levelNumber)
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
void AssetsManager::setScoreScreenPalette()
{
	// From OpenClaw
	static vector<uint8_t> g_ScoreScreenPalette = {
			0x00, 0x00, 0x00, // 0
			255, 252, 202, // 1 - DONE
			240, 238, 90, // 2 - DONE
			223, 220, 76, // 3 - DONE
			219, 217, 74, // 4 - DONE
			210, 208, 72, // 5 - DONE
			189, 187, 63, // 6 - DONE
			154, 153, 52, // 7 - DONE
			130, 115, 44, // 8 - DONE
			98, 97, 33, // 9 - DONE
			64, 63, 21, // 10 - DONE
			41, 41, 14, // 11 - DONE
			38, 35, 44, // 12 - DONE
			13, 11, 113, // 13 - DONE
			19, 19, 170, // 14 - DONE
			0, 104, 144, // 15 - DONE
			178, 187, 188, // 16 - DONE
			18, 157, 18, // 17 - DONE
			75, 74, 68, // 18 - DONE
			112, 111, 105, // 19 - DONE
			220, 96, 0, // 20 - DONE
			99, 4, 0, // 21 - DONE
			138, 138, 138, // 22 - DONE
			125, 41, 0, // 23 - DONE
			44, 44, 44, // 24 - DONE
			53, 6, 0, // 25 - DONE  
			253, 247, 202, // 26 - DONE
			0xFE, 0xF4, 0x91, // 27 - DONE
			252, 236, 67, // 28 - DONE
			247, 232, 11, // 29 - DONE
			240, 212, 95, // 30 - DONE
			224, 183, 31, // 31 - DONE
			205, 156, 15, // 32 - DONE
			190, 134, 18, // 33 - DONE
			250, 238, 140, // 34 - DONE
			239, 222, 97, // 35 - DONE
			217, 201, 90, // 36 - DONE
			194, 179, 78, // 37 - DONE
			161, 149, 65, // 38 - DONE
			129, 119, 52, // 39 - DONE
			91, 0, 18, // 40 - DONE
			251, 221, 29, // 41 - DONE
			247, 227, 121, // 42 - DONE
			219, 136, 15, // 43 - DONE
			171, 73, 9, // 44 - DONE
			125, 38, 17, // 45 - DONE
			219, 217, 254, // 46 - DONE
			143, 137, 251, // 47 - DONE
			45, 40, 218, // 48 - DONE
			7, 6, 182, // 49 - DONE
			172, 251, 203, // 50 - DONE
			21, 233, 60, // 51 - DONE
			0, 158, 0, // 52 - DONE
			0, 48, 0, // 53 - DONE
			243, 199, 254, // 54 - DONE
			0xFF, 0x00, 0xFF, // 55                 55
			0x5B, 0x5F, 0xFF, // 56                 56
			0x97, 0x9C, 0xFF, // 57                 57
			254, 193, 194, // 58 - DONE
			253, 56, 57, // 59 - DONE
			247, 2, 3, // 60 - DONE
			146, 11, 24, // 61 - DONE
			250, 222, 109, // 62 - DONE
			251, 239, 191, // 63 - DONE
			242, 198, 81, // 64 - DONE
			243, 187, 48, // 65 - DONE
			205, 138, 15, // 66 - DONE
			232, 152, 13, // 67 - DONE
			157, 101, 24, // 68 - DONE
			221, 150, 53, // 69 - DONE
			199, 138, 57, // 70 - DONE
			196, 112, 25, // 71 - DONE
			143, 67, 10, // 72 - DONE
			79, 36, 5, // 73 - DONE     
			0, 255, 156, // 74 - DONE
			8, 48, 121, // 75 - DONE
			255, 0, 255, // 76 - DONE
			90, 10, 59, // 77 - DONE
			250, 234, 152, // 78 - DONE
			241, 188, 0, // 79 - DONE
			238, 189, 37, // 80 - DONE
			209, 117, 5, // 81 - DONE
			145, 72, 6, // 82 - DONE
			11, 174, 11, // 83 - DONE
			127, 211, 129, // 84 - DONE
			112, 50, 3, // 85 - DONE
			9, 86, 0, // 86 - DONE
			70, 208, 46, // 87 - DONE
			0xF9, 0xDB, 0x42, // 88                 88
			232, 251, 212, // 89 - DONE
			86, 219, 212, // 90 - DONE
			0xF0, 0x58, 0x02, // 91
			34, 96, 250, // 92 - DONE
			18, 100, 28, // 93 - DONE
			55, 157, 56, // 94 - DONE
			7, 52, 3, // 95 - DONE
			120, 208, 110, // 96 - DONE
			192, 109, 134, // 97 - DONE
			197, 45, 93, // 98 - DONE
			145, 18, 58, // 99 - DONE
			95, 64, 28, // 100 - DONE
			110, 74, 32, // 101 - DONE
			18, 8, 43, // 102 - DONE
			41, 0, 0, // 103 - DONE
			106, 57, 8, // 104 - DONE
			0xF3, 0xC3, 0x4D, // 105
			0xF4, 0xD7, 0x5C, // 106
			0xD7, 0xA8, 0x18, // 107
			115, 90, 41, // 108 - DONE
			115, 82, 33, // 109 - DONE
			115, 75, 24, // 110 - DONE
			114, 75, 16, // 111 - DONE
			112, 66, 14, // 112 - DONE
			109, 57, 16, // 113 - DONE
			0x9A, 0x11, 0xC5, // 114                114
			0x05, 0xFF, 0xA1, // 115                115
			108, 49, 9, // 116 - DONE
			99, 49, 16, // 117 - DONE
			0xEC, 0x75, 0xFF, // 118                118
			0xF8, 0x9F, 0xFF, // 119                119
			90, 49, 5, // 120 - DONE
			100, 41, 5, // 121 - DONE
			90, 41, 8, // 122 - DONE
			90, 41, 0, // 123 - DONE
			81, 42, 8, // 124 - DONE
			0xBC, 0x08, 0x7D, // 125                125
			91, 32, 2, // 126 - DONE
			80, 33, 8, // 127 - DONE
			82, 33, 0, // 128 - DONE
			0x9A, 0x53, 0x16, // 129                129
			0x90, 0x40, 0x0D, // 130                130
			82, 24, 0, // 131 - DONE
			79, 24, 8, // 132 - DONE
			74, 24, 0, // 133 - DONE
			65, 24, 0, // 134 - DONE
			75, 16, 0, // 135 - DONE
			61, 16, 0, // 136 - DONE
			43, 15, 0, // 137 - DONE
			0xD4, 0x03, 0xC8, // 138                138
			0xB8, 0x06, 0xAB, // 139                139
			0x9C, 0x0C, 0x83, // 140                140
			0x79, 0x05, 0x73, // 141                141
			0x68, 0x03, 0x65, // 142                142
			0x52, 0x00, 0x5B, // 143                143
			0x9A, 0x9B, 0x9B, // 144                144
			0xAB, 0xAB, 0xAB, // 145                145
			0x47, 0x30, 0x13, // 146                146
			0x5D, 0x3C, 0x14, // 147                147
			0x71, 0x4B, 0x14, // 148                148
			0x69, 0x42, 0x1D, // 149                149
			0x51, 0x32, 0x14, // 150                150
			10, 10, 10, // 151 - DONE
			9, 9, 9, // 152 - DONE
			0x38, 0x2E, 0x2E, // 153                153
			0x61, 0x53, 0x48, // 154                154
			0x88, 0x79, 0x60, // 155                155
			130, 130, 130, // 156 - DONE
			0xBC, 0x74, 0x83, // 157                157
			100, 100, 100, // 158 - DONE
			0x5E, 0x3E, 0x71, // 159                159
			0x03, 0x21, 0x36, // 160                160
			77, 77, 77, // 161 - DONE
			0x03, 0x4F, 0x74, // 162                START
			0x03, 0x59, 0x7F, // 163
			58, 58, 58, // 164 - DONE
			0x03, 0x75, 0xA3, // 165
			0x03, 0x85, 0xBB, // 166
			0x03, 0x94, 0xC3, // 167
			0x00, 0x27, 0x4E, // 168
			8, 8, 8, // 169 - DONE
			114, 70, 88, // 170 - DONE
			0x03, 0x03, 0x03, // 171
			0x57, 0xA9, 0xC1, // 172
			0x3F, 0x88, 0xA8, // 173
			0x24, 0x56, 0x79, // 174
			0x18, 0x39, 0x54, // 175
			0x11, 0x25, 0x37, // 176                END
			123, 132, 0, // 177 - DONE
			97, 97, 7, // 178 - DONE
			102, 91, 23, // 179 - DONE
			137, 118, 32, // 180 - DONE
			186, 176, 141, // 181 - DONE
			134, 103, 3, // 182 - DONE
			0x03, 0x07, 0x0B, // 183                183
			0x02, 0x05, 0x08, // 184                184
			148, 90, 0, // 185 - DONE
			0x00, 0x01, 0x02, // 186                186
			214, 135, 30, // 187 - DONE
			240, 154, 52, // 188 - DONE
			0x66, 0x0F, 0x08, // 189                189
			215, 123, 26, // 190 - DONE
			192, 109, 22, // 191 - DONE
			214, 129, 41, // 192 - DONE
			206, 109, 13, // 193 - DONE
			221, 145, 71, // 194 - DONE
			179, 91, 6, // 195 - DONE
			241, 139, 41, // 196 - DONE
			214, 110, 13, // 197 - DONE
			236, 127, 30, // 198 - DONE
			189, 92, 7, // 199 - DONE
			206, 110, 26, // 200 - DONE
			214, 111, 26, // 201 - DONE
			210, 123, 51, // 202 - DONE
			226, 112, 21, // 203 - DONE
			202, 93, 7, // 204 - DONE
			178, 82, 7, // 205 - DONE
			201, 111, 41, // 206 - DONE
			135, 99, 71, // 207 - DONE
			170, 74, 0, // 208 - DONE
			178, 86, 16, // 209 - DONE
			189, 89, 16, // 210 - DONE
			182, 91, 24, // 211 - DONE
			154, 67, 5, // 212 - DONE
			198, 92, 16, // 213 - DONE
			189, 82, 7, // 214 - DONE
			228, 113, 33, // 215 - DONE
			170, 74, 8, // 216 - DONE
			210, 94, 16, // 217 - DONE
			198, 96, 27, // 218 - DONE
			168, 66, 0, // 219 - DONE
			178, 93, 38, // 220 - DONE
			155, 115, 89, // 221 - DONE
			212, 97, 25, // 222 - DONE
			185, 74, 6, // 223 - DONE
			143, 73, 30, // 224 - DONE
			136, 50, 0, // 225 - DONE
			170, 74, 19, // 226 - DONE
			168, 66, 8, // 227 - DONE
			152, 54, 0, // 228 - DONE
			135, 51, 8, // 229 - DONE
			189, 74, 17, // 230 - DONE
			103, 34, 0, // 231 - DONE
			125, 41, 0, // 232 - DONE
			168, 66, 17, // 233 - DONE
			152, 54, 8, // 234 - DONE
			185, 62, 8, // 235 - DONE
			167, 54, 5, // 236 - DONE
			144, 41, 0, // 237 - DONE
			87, 24, 0, // 238 - DONE
			134, 51, 19, // 239 - DONE
			68, 18, 0, // 240 - DONE
			115, 30, 0, // 241 - DONE
			129, 32, 0, // 242 - DONE
			129, 38, 8, // 243 - DONE
			103, 24, 0, // 244 - DONE
			96, 31, 15, // 245 - DONE
			82, 15, 0, // 246 - DONE
			94, 16, 0, // 247 - DONE
			61, 25, 20, // 248 - DONE
			69, 8, 0, // 249 - DONE
			53, 6, 0, // 250 - DONE
			34, 3, 0, // 251 - DONE
			0x00, 0x53, 0x98, // 252                252
			8, 0, 0, // 253 - DONE
			123, 49, 49, // 254 - DONE
			0xFF, 0xFF, 0xFF, // 255                255
	};

	_imagesManager->setPalette(allocNewSharedPtr<PidPalette>(g_ScoreScreenPalette));
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
	}
}
