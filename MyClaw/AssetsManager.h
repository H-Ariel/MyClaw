#pragma once

#include "AnimationsManager.h"
#include "ImagesManager.h"
#include "AudioManager.h"
#include "WwdFile.h"


class AssetsManager
{
public:
	static void Initialize();
	static void Finalize();

	static shared_ptr<UIBaseImage> loadImage(const string& path);
	static shared_ptr<Animation> loadAnimation(const string& aniPath, const string& imageSetPath = ""); // load from ANI file
	static shared_ptr<Animation> loadCopyAnimation(const string& aniPath, const string& imageSetPath = "");
	static shared_ptr<Animation> createAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder); // create new animation from directory content
	static shared_ptr<Animation> createCopyAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder);
	static shared_ptr<Animation> createAnimationFromFromPidImage(const string& pidPath); // convert PID image to an animtion
	static shared_ptr<Animation> createCopyAnimationFromFromPidImage(const string& pidPath);
	static map<string, shared_ptr<Animation>> loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath = ""); // load all ANI files in `dirPath`
	static shared_ptr<WapWorld> loadWwdFile(const string& wwdPath);
	static shared_ptr<WapWorld> loadLevelWwdFile(int8_t levelNumber);
	static shared_ptr<PidPalette> loadPidPalette(const string& palPath);
	static map<int32_t, shared_ptr<UIBaseImage>> loadPlaneTilesImages(const string& planeImagesPath);

	static shared_ptr<MidiPlayer> getMidiPlayer(const string& xmiFilePath);

	static uint32_t playWavFile(const string& wavFilePath, int32_t volume = 100, bool infinite = false); // volume range is [0,100]. return wav player id
	static void stopWavFile(uint32_t wavFileId);
	static uint32_t getWavFileDuration(uint32_t wavFileId);
	static void setBackgroundMusic(AudioManager::BackgroundMusicType type, bool reset);

	static void callLogics(uint32_t elapsedTime);

	static void clearLevelAssets(int lvl);

private:
	static RezArchive* _rezArchive;
	static ImagesManager* _imagesManager;
	static AnimationsManager* _animationsManager;
	static AudioManager* _audioManager;
	static bool _runApp;
};
