#pragma once

#include "AnimationsManager.h"
#include "PathManager.h"
#include "RezParser/PalFile.h"
#include "RezParser/WwdFile.h"


class AssetsManager
{
public:
	enum class BackgroundMusicType : int8_t
	{
		None,
		Level,
		Powerup,
		Boss,
		Credits
	};

	static void Initialize();
	static void Finalize();

	static shared_ptr<UIBaseImage> loadImage(const string& path, const vector<ColorF>* colors = nullptr);
	static map<int, shared_ptr<UIBaseImage>> loadPlaneTilesImages(const string& planeImagesPath);

	// Loads a UIAnimation from an ANI file.
	static shared_ptr<UIAnimation> loadAnimation(const string& aniPath, const string& imageSetPath = "");
	static shared_ptr<UIAnimation> loadCopyAnimation(const string& aniPath, const string& imageSetPath = "");

	// Creates a new animation from the contents of a directory.
	static shared_ptr<UIAnimation> createAnimationFromDirectory(const string& dirPath, bool reversedOrder = false, uint32_t duration = 100);
	static shared_ptr<UIAnimation> createCopyAnimationFromDirectory(const string& dirPath, bool reversedOrder = false, uint32_t duration = 100);

	// Converts a PID image into a UIAnimation.
	static shared_ptr<UIAnimation> createAnimationFromPidImage(const string& pidPath);
	static shared_ptr<UIAnimation> createCopyAnimationFromPidImage(const string& pidPath);

	// Loads all ANI files from `dirPath`.
	// If `colors` is not null, each image includes shadows in all colors, in addition to the original image.
	// (images type is UIColorfullyImage
	static unordered_map<string, shared_ptr<UIAnimation>> loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath = "", const vector<ColorF>* colors = nullptr);
	
	
	static shared_ptr<WapWwd> loadLevel(int levelNumber); // load and init level, return its WWD
	static void loadPidPalette(const string& palPath); // load the palette and set it to the images manager
	static string getCreditsText();

	static uint32_t playWavFile(const string& wavFilePath, int volume = 100, bool infinite = false); // volume range is [0,100]. return wav player id
	static void stopWavFile(uint32_t wavId);
	static uint32_t getWavFileDuration(const string& wavFileKey);
	static void startBackgroundMusic(BackgroundMusicType type);
	static void stopBackgroundMusic();

	static void callLogics(uint32_t elapsedTime); // calls logics for all animations and sounds

	static void clearLevelAssets();

	static void applySettings();


	static const uint32_t INVALID_AUDIOPLAYER_ID;

private:
	static AssetsManager* instance;


	AssetsManager();
	~AssetsManager();

	void loadWavDir(const string& path); // load all wav files from directory

	
	WapPal _palette; // current palette
	map<AssetsManager::BackgroundMusicType, uint32_t> bgMusics; // [type]=audio-player-id
	RezArchive _rezArchive;
	AnimationsManager* _animationsManager;
	BackgroundMusicType _lastType; // last played background music type
	bool _savePcxPalette;
};
