#pragma once

#include "IAudioPlayer.h"
#include "Framework/Buffer.h"


class AudioManager
{
public:
	static void Initialize();
	static void Finalize();

	static void addWavPlayer(const string& key, const DynamicArray<uint8_t>& wav);
	static void addMidiPlayer(const string& key, const DynamicArray<uint8_t>& midi);

	static uint32_t playWav(const string& key, bool infinite, int volume = 100); // volume range is [0,100]
	static uint32_t playMidi(const string& key, bool infinite);

	static void stopWav(uint32_t id);
	static void removeWav(uint32_t id);

	static void stopMidi(uint32_t id);
	static void removeMidi(uint32_t id);

	static void remove(function<bool(const string& key)> predicate);

	static uint32_t getWavDuration(const string& key);
	static void setVolume(uint32_t id, int volume); // set the volume. value range is [0,100]


	static const uint32_t INVALID_ID = 0;

private:
	static AudioManager* instance;

	AudioManager();

	uint32_t getNewMidiId(); // return new id of audio player

	map<uint32_t, shared_ptr<IAudioPlayer>> _midiPlayers; // [key]=audio-player
	map<string, DynamicArray<uint8_t>> _midiDataCache; // [key]=data
};
