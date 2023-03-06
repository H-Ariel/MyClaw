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

	static shared_ptr<UIBaseImage> loadImage(string path);
	static shared_ptr<Animation> loadAnimation(string aniPath, string imageSetPath = ""); // load from ANI file
	static shared_ptr<Animation> loadCopyAnimation(string aniPath, string imageSetPath = "");
	static shared_ptr<Animation> createAnimationFromDirectory(string dirPath, uint32_t duration = 125, bool reversedOrder = false); // create new animation from directory content
	static shared_ptr<Animation> createCopyAnimationFromDirectory(string dirPath, uint32_t duration = 125, bool reversedOrder = false);
	static shared_ptr<Animation> createAnimationFromFromPidImage(string pidPath); // convert PID image to an animtion
	static shared_ptr<Animation> createCopyAnimationFromFromPidImage(string pidPath);
	static map<string, shared_ptr<Animation>> loadAnimationsFromDirectory(string dirPath, string imageSetPath = ""); // load all ANI files in `dirPath`
	static shared_ptr<WapWorld> loadWwdFile(string wwdPath);
	static shared_ptr<PidPalette> loadPidPalette(string palPath);
	static map<int32_t, shared_ptr<UIBaseImage>> loadPlaneTilesImages(string planeImagesPath);

	static shared_ptr<MidiPlayer> getMidiPlayer(string xmiFilePath);

	static void playWavFile(string wavFilePath, bool infinite = false);
	static void stopWavFile(string wavFilePath);
	static uint32_t getWavFileDuration(string wavFilePath);
	static void setWavFileVolume(string wavFilePath, int32_t volume); // set the volume. value range is [0,100]
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
