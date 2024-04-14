#pragma once

#include "IAudioPlayer.h"
#include "Framework/BufferReader.h"


class AudioManager
{
public:
	static void Initialize();
	static void Finalize();

	static void addWavPlayer(const string& key, shared_ptr<BufferReader> wavReader);
	static void addMidiPlayer(const string& key, const vector<uint8_t>& midi);

	static uint32_t playWav(const string& key, bool infinite);
	static uint32_t playMidi(const string& key, bool infinite);

	static void stop(uint32_t id);
	static void continuePlay(uint32_t id);
	static void remove(uint32_t id);

	static uint32_t getDuration(const string& key);
	static void setVolume(uint32_t id, int volume); // set the volume. value range is [0,100]

	static void checkForRestart();

	static void remove(function<bool(const string& key)> predicate);

	static const uint32_t INVALID_ID = UINT32_MAX;

private:
	static AudioManager* instance;

	AudioManager();

	uint32_t getNewId(); // return new id of audio player


	map<uint32_t, shared_ptr<IAudioPlayer>> _audioPlayers; // [key]=audio-player
	map<string, tuple<WAVEFORMATEX, vector<uint8_t>>> _audioDataCache; // [key]=(fmt, data) | if its MIDI data, fmt is empty
};
