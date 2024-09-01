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
	static void remove(uint32_t id);
	static void remove(function<bool(const string& key)> predicate);

	static uint32_t getDuration(const string& key);
	static void setVolume(uint32_t id, int volume); // set the volume. value range is [0,100]


	static const uint32_t INVALID_ID = 0;

private:
	static AudioManager* instance;

	static const int NUM_OF_PLAYING_THREADS = 6;

	AudioManager();
	~AudioManager();

	uint32_t getNewId(); // return new id of audio player
	void playFromQueue();


	map<uint32_t, shared_ptr<IAudioPlayer>> _audioPlayers; // [key]=audio-player
	map<string, tuple<WAVEFORMATEX, vector<uint8_t>>> _audioDataCache; // [key]=(fmt, data) | if its MIDI data, fmt is empty
	queue<pair<shared_ptr<IAudioPlayer>, bool>> audioQueue; // (player, infinite)
	mutex audioQueueMutex; // mutex for audioQueue

	thread* threads[NUM_OF_PLAYING_THREADS];
	bool running;
};
