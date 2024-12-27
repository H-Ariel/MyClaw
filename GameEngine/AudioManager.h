#pragma once

#include "MidiPlayer.h"
#include "Framework/DynamicArray.hpp"


/// <summary>
/// Manages audio playback, including WAV and MIDI formats.
/// </summary>
class AudioManager
{
public:
	/// <summary>
	/// Initializes the AudioManager, preparing resources for audio playback.
	/// </summary>
	static void Initialize();

	/// <summary>
	/// Finalizes the AudioManager, releasing all resources.
	/// </summary>
	static void Finalize();

	/// <summary>
	/// Adds a WAV player to the AudioManager.
	/// </summary>
	/// <param name="key">A unique identifier for the WAV player.</param>
	/// <param name="wav">The WAV audio data as a DynamicArray of bytes.</param>
	static void addWavPlayer(const string& key, const DynamicArray<uint8_t>& wav);

	/// <summary>
	/// Adds a MIDI player to the AudioManager.
	/// </summary>
	/// <param name="key">A unique identifier for the MIDI player.</param>
	/// <param name="midi">The MIDI audio data as a DynamicArray of bytes.</param>
	static void addMidiPlayer(const string& key, const DynamicArray<uint8_t>& midi);

	/// <summary>
	/// Starts playing a WAV file.
	/// </summary>
	/// <param name="key">The key for the WAV file.</param>
	/// <param name="infinite">True if the audio should play infinitely, false otherwise.</param>
	/// <param name="volume">The volume of the audio, in the range [0,1].</param>
	/// <returns>An ID for the playing WAV instance.</returns>
	static uint32_t playWav(const string& key, bool infinite, float volume = 1);

	/// <summary>
	/// Starts playing a MIDI file.
	/// </summary>
	/// <param name="key">The key for the MIDI file.</param>
	/// <param name="infinite">True if the audio should play infinitely, false otherwise.</param>
	/// <returns>An ID for the playing MIDI instance.</returns>
	static uint32_t playMidi(const string& key, bool infinite);

	/// <summary>
	/// Stops a currently playing WAV audio by its ID.
	/// </summary>
	/// <param name="id">The ID of the WAV audio instance to stop.</param>
	static void stopWav(uint32_t id);

	/// <summary>
	/// Stops a currently playing MIDI audio by its ID.
	/// </summary>
	/// <param name="id">The ID of the MIDI audio instance to stop.</param>
	static void stopMidi(uint32_t id);

	/// <summary>
	/// Removes a MIDI player by its ID.
	/// </summary>
	/// <param name="id">The ID of the MIDI player to remove.</param>
	static void removeMidi(uint32_t id);

	/// <summary>
	/// Sets the global volume for all WAV audio playback.
	/// </summary>
	/// <param name="volume">The global volume, in the range [0,1].</param>
	static void setWavVolume(float volume);

	/// <summary>
	/// Sets the global volume for all MIDI audio playback.
	/// </summary>
	/// <param name="volume">The global volume, in the range [0,1].</param>
	static void setMidiVolume(float volume);

	/// <summary>
	/// Removes WAV and MIDI players by key, using a filtering function.
	/// </summary>
	/// <param name="filter">A function that returns true for the keys to be removed.</param>
	static void remove(function<bool(const string& key)> filter);

	/// <summary>
	/// Gets the duration of a WAV sound in milliseconds.
	/// </summary>
	/// <param name="key">The key for the WAV sound.</param>
	/// <returns>The duration of the WAV in milliseconds.</returns>
	static uint32_t getWavDuration(const string& key);

	/// <summary>
	/// Represents an invalid ID (0).
	/// </summary>
	static const uint32_t INVALID_ID = 0;

private:
	/// <summary>
	/// A static pointer to the single instance of the AudioManager (Singleton pattern).
	  /// </summary>
	static AudioManager* instance;

	/// <summary>
	/// Constructor for the AudioManager. Initializes the necessary components for audio management.
	/// </summary>
	AudioManager();

	/// <summary>
	/// Destructor for the AudioManager. Cleans up resources and stops all audio playback.
	/// </summary>
	~AudioManager();


	/// <summary>
	/// Generates a new unique ID for a MIDI player.
	/// </summary>
	/// <returns>A new MIDI ID.</returns>
	uint32_t getNewMidiId();

	/// <summary>
	/// Generates a new unique ID for a WAV audio player.
	/// </summary>
	/// <returns>A new WAV ID.</returns>
	uint32_t getNewWavId();

	/// <summary>
	/// Matches the WAV format with the audio data.
	/// </summary>
	/// <param name="fmt">The WAVEFORMATEX structure to match the format.</param>
	/// <param name="wavSoundData">The WAV audio data.</param>
	/// <returns>A DynamicArray containing the matched audio data.</returns>
	DynamicArray<int16_t> matchWavFormat(WAVEFORMATEX& fmt, const DynamicArray<uint8_t>& wavSoundData) const;

	/// <summary>
	/// Mixes multiple WAV audios into a single stream.
	/// </summary>
	/// <param name="stream">The mixed audio stream.</param>
	/// <param name="len">The length of the stream.</param>
	/// <returns>The number of samples mixed.</returns>
	int mixWavAudios(int16_t* stream, int len);

	/// <summary>
	/// Starts the background thread for playing WAV audio.
	/// </summary>
	void playWavAudioThread();

	// MIDI related fields:
	map<uint32_t, shared_ptr<MidiPlayer>> _midiPlayers; // MIDI players map
	map<string, DynamicArray<uint8_t>> _midiDataCache; // MIDI data cache
	float midiGlobalVolume; // Global volume for MIDI

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

	// WAV related fields:
	map<string, WavAudioData> savedWavAudios; // Saved WAV audio data
	list<WavAudioData> wavAudios; // List of currently playing WAV audios
	mutex wavAudiosListMutex; // Mutex for managing concurrent access to WAV list
	WAVEFORMATEX WavFormat; // WAV format, initialized in constructor
	IDirectSound8* dSound; // DirectSound instance for WAV playback
	IDirectSoundBuffer* primaryBuffer; // Primary DirectSound buffer
	thread* t_play; // Background thread for WAV playback
	float wavGlobalVolume; // Global volume for WAV
	bool running; // Flag indicating whether audio is running
};
