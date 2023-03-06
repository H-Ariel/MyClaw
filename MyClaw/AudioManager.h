#pragma once

#include "WavPlayer.h"
#include "MidiPlayer.h"


class AudioManager
{
public:
	AudioManager(RezArchive* rezArchive);
	~AudioManager();

	void playWavFile(string wavFilePath, bool infinite);
	void stopWavFile(string wavFilePath);
	uint32_t getWavFileDuration(string wavFilePath);

	// set the volume. value range is [0,100]
	void setVolume(string wavFilePath, int32_t volume);

	void clearLevelSounds(string prefix);

	void checkForRestart();

	enum class BackgroundMusicType
	{
		None,
		Level,
		Powerup,
		Boss
	};
	void setBackgroundMusic(BackgroundMusicType type, bool reset);


private:
	shared_ptr<WavPlayer> getWavPlayer(string wavFilePath);

	map<string, shared_ptr<WavPlayer>> _wavPlayers; // [path]=player
	map<BackgroundMusicType, shared_ptr<MidiPlayer>> _midiPlayers; // [type]=player
	shared_ptr<MidiPlayer> _currBgMusic;
	BackgroundMusicType _currBgMusicType;
	RezArchive* _rezArchive;
};
