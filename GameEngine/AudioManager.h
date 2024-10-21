#pragma once

#include "MidiPlayer.h"
#include "Framework/Buffer.h"


// TODO: add WAV to signatures if needed


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

	static void stopMidi(uint32_t id);
	static void removeMidi(uint32_t id);

	// remove WAV and MIDI by their key
	static void remove(function<bool(const string& key)> predicate);

	// returns the duration of the sound in milliseconds for WAV files
	static uint32_t getWavDuration(const string& key);


	static const uint32_t INVALID_ID = 0;

private:
	static AudioManager* instance;

	AudioManager();
	~AudioManager();

	uint32_t getNewMidiId(); // return new id for midi players
	uint32_t getNewWavId(); // return new id for wav audios

	DynamicArray<int16_t> matchWavFormat(WAVEFORMATEX& fmt, const DynamicArray<uint8_t>& wavSoundData) const;
	int mixWavAudios(int16_t* stream, int len);
	void playWavAudioThread();


	// MIDI fields

	map<uint32_t, shared_ptr<MidiPlayer>> _midiPlayers; // [key]=audio-player
	map<string, DynamicArray<uint8_t>> _midiDataCache; // [key]=data


	struct WavAudioData {
		int16_t* soundBuffer;
		int soundBufferLength;
		int16_t* currPtr; // pointer to current data in `soundBuffer`
		int currLength;   // remains bytes (matches with currPtr)
		float volume;
		uint32_t id;
		bool infinite;

		// sets currPtr and currLength to buffer's beginning
		void reset() { currPtr = soundBuffer; currLength = soundBufferLength; }
	};

	// WAV fields:

	map<string, WavAudioData> savedWavAudios; // [key]=wav
	list<WavAudioData> wavAudios;
	mutex wavAudiosListMutex; // mutex for `list<> wavAudios`
	WAVEFORMATEX WavFormat; // init in ctor. do not change. // TODO: make static
	IDirectSound8* dSound;
	IDirectSoundBuffer* primaryBuffer;
	thread* t_play;
	float wavGlobalVolume; // volume multiplier for all wavAudios - 0.0 to 1.0
	bool running;
};
