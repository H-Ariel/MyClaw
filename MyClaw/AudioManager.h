#pragma once

#include "WavPlayer.h"


class MidiPlayer;


class AudioManager
{
public:
	AudioManager(RezArchive* rezArchive);
	~AudioManager();

	uint32_t playWavFile(string wavFilePath, bool infinite); // create wav player and returns its ID
	void stopWavFile(uint32_t wavFileId);
	uint32_t getWavFileDuration(uint32_t wavFileId);
	void setVolume(uint32_t wavFileId, int32_t volume); // set the volume. value range is [0,100]

	void clearLevelSounds();

	void checkForRestart();

	enum class BackgroundMusicType
	{
		None,
		Level,
		Powerup,
		Boss
	};
	void setBackgroundMusic(BackgroundMusicType type);


private:
	uint32_t getNewId();

	map<uint32_t, shared_ptr<WavPlayer>> _wavPlayers; // [id]=player
	map<BackgroundMusicType, shared_ptr<MidiPlayer>> _midiPlayers; // [type]=player
	shared_ptr<MidiPlayer> _currBgMusic;
	BackgroundMusicType _currBgMusicType;
	RezArchive* _rezArchive;
};
