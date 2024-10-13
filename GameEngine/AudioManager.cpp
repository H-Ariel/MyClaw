#include "AudioManager.h"
#include "MidiPlayer.h"

#include <SDL2/SDL.h>


AudioManager* AudioManager::instance = nullptr;


static int clamp(int value, int min, int max) {
	return value < min ? min : value > max ? max : value;
}

// Audio Mixer. for now only for WAV files in specific format.
// TODO: use C WinApi `waveOut...` instead of SDL2 (or something which not outside-library)
class AudioMixer {
public:
	AudioMixer() : volume(1) {
		// set this_wavSpec values:
		this_wavSpec = {};
		this_wavSpec.freq = 44100;
		this_wavSpec.format = AUDIO_S16;
		this_wavSpec.channels = 1;
		this_wavSpec.userdata = this;
		this_wavSpec.callback = audioCallback;

		if (SDL_OpenAudio(&this_wavSpec, NULL) < 0) {
			//throw Exception("Error: Could not open audio");
			cout << "Error: Could not open audio - " << SDL_GetError() << endl;
			// TODO: handle errors better
		}
		else {
			SDL_PauseAudio(0);
		}
	}
	~AudioMixer() {
		cleanup();	
		SDL_CloseAudio();
	}

	// if success returns `id`, returns INVALID_ID.
	uint32_t playWav(const string& key, float volume, bool infinite) {
		auto it = savedAudios.find(key);
		if (it == savedAudios.end())
		{
			LOG("[Warning] " __FUNCTION__ ": key not found. key: %s", key.c_str());
			return AudioManager::INVALID_ID;
		}
		AudioData audioData = savedAudios[key];
		audioData.reset();
		audioData.volume = volume;
		audioData.id = 0; // TODO: set real id
		audioData.infinite = infinite;
		audios.push_back(audioData);
		return audioData.id;
	}

	void addWavBuffer(const DynamicArray<uint8_t>& wavBuf, const string& key) {
		if (savedAudios.find(key) != savedAudios.end()) return; // already exists

		AudioData audio = {};
		Uint8* audioBuffer = nullptr;
		Uint32 length = 0;
		SDL_AudioSpec wavSpec;
		SDL_RWops* rw = SDL_RWFromConstMem(wavBuf.data(), (int)wavBuf.size());
		auto ret = SDL_LoadWAV_RW(rw, 1, &wavSpec, &audioBuffer, &length);
		SDL_FreeRW(rw);
		if (ret == NULL)
		{
			// TODO: handle errors better
		//	throw Exception("Error: Could not load WAV data from memory");
			cout << "ERROR at " __FUNCTION__ ": key=" << key << endl;
			cout << "SDL_GetError() = " << SDL_GetError() << endl;
			return;
		}

		// Convert `wavSpec` to be same as `this_wavSpec`
		SDL_AudioCVT cvt;
		SDL_BuildAudioCVT(&cvt, wavSpec.format, wavSpec.channels, wavSpec.freq,
			this_wavSpec.format, this_wavSpec.channels, this_wavSpec.freq);
		if (cvt.needed) {
			cvt.len = length;
			cvt.buf = (Uint8*)SDL_malloc((size_t)(cvt.len * cvt.len_mult));
			SDL_memcpy(cvt.buf, audioBuffer, length);
			SDL_ConvertAudio(&cvt);
			SDL_FreeWAV(audioBuffer);
			audio.audioBuffer = (Sint16*)cvt.buf;
			audio.length = cvt.len_cvt / 2; // `cvt.len_cvt` contains amount of total bytes. `length` is for `audioBuffer` (16 bits (2 bytes) buffer).
		}
		else {
			audio.audioBuffer = (Sint16*)audioBuffer;
			audio.length = length;
		}

		audio.key = key;
		savedAudios[key] = audio;
	}

	float getWavDuration(const string& key) {
		auto it = savedAudios.find(key);
		if (it != savedAudios.end()) {
			const AudioData& audioData = it->second;
			// Calculate duration in seconds
			float duration = static_cast<float>(audioData.length) / (sizeof(Sint16) * this_wavSpec.freq);
			return duration;
		}
		// Return -1.0 to indicate that the key was not found
		return -1.0f;
	}

	void stop(uint32_t id) {
		remove(id);
	}

	void remove(uint32_t id) {
		audios.remove_if([&](const AudioData& a) { return a.id == id; });
	}

	void remove(function<bool(const string& key)> predicate)
	{
		for (auto it = savedAudios.begin(); it != savedAudios.end();)
		{
			if (predicate(it->first)) {
				audios.remove_if([&](const AudioData& a) { return a.audioBuffer == it->second.audioBuffer; }); // remove the buffer we freed

				SDL_FreeWAV((Uint8*)it->second.audioBuffer);
				it = savedAudios.erase(it);
			}
			else
				++it;
		}
	}

private:
	struct AudioData {
		Sint16* audioBuffer; // the audio buffer. this buffer filled by SDL at `addWavBuffer`.
		Uint32 length;     // audio buffer's length (bytes' count) (filled as audioBuffer)
		Sint16* currPtr;   // pointer to current data
		Uint32 currLength; // remains bytes (matches with currPtr)
		float volume;      // volume multiplier for this audio - 0.0 to 1.0
		uint32_t id; // TODO: use `audioBuffer` as id ...
		bool infinite;
		string key; // TODO delete this. just for debug

		AudioData()
			: audioBuffer(nullptr), length(0), currPtr(nullptr), 
			currLength(0), volume(1), id(0), infinite(false) { }

		void reset() {
			currPtr = (Sint16*)audioBuffer;
			currLength = length;
		}
	};

	SDL_AudioSpec this_wavSpec;
	map<string, AudioData> savedAudios; // [key]=wav
	list<AudioData> audios;
	float volume; // volume multiplier for all audios - 0.0 to 1.0


	// callback function to mix all playing audios to one stream
	static void audioCallback(void* userdata, Uint8* stream, int len) {
		AudioMixer* mixer = static_cast<AudioMixer*>(userdata);
		Sint16* output = (Sint16*)stream; // Cast stream to Sint16 for 16-bit audio
		len /= 2; // Since each sample is now 2 bytes (16-bit)

		int* sum = DBG_NEW int[len]; // sum of voices in each index
		for (int i = 0; i < len; sum[i++] = 0);

		for (auto it = mixer->audios.begin(); it != mixer->audios.end(); ) {
			auto& audio = *it;
			if (audio.currLength == 0) {
				if (audio.infinite)
					audio.reset();
				else
					it = mixer->audios.erase(it);
			}
			else {
				Uint32 lengthToMix = min((Uint32)len, audio.currLength);

				for (Uint32 j = 0; j < lengthToMix; j++)
					sum[j] += (int)(audio.currPtr[j] * audio.volume); // apply volume scaling

				audio.currPtr += lengthToMix;
				audio.currLength -= lengthToMix;

				++it;
			}
		}

		for (int i = 0; i < len; i++) {
			int val = (int)(sum[i] * mixer->volume);
			output[i] = (Sint16)clamp(val, INT16_MIN, INT16_MAX); // Clamp to 16-bit signed range
		}

		delete[] sum;
	}

	// clean all saved-audios
	void cleanup() {
		for (auto& it : savedAudios)
			SDL_FreeWAV((Uint8*)it.second.audioBuffer);
		savedAudios.clear();
	}
};


AudioMixer audioMixer;


void AudioManager::Initialize()
{
	if (instance == nullptr)
		instance = DBG_NEW AudioManager();
}
void AudioManager::Finalize()
{
	if (instance)
	{
		delete instance;
		instance = nullptr;
	}
}


AudioManager::AudioManager() { }

uint32_t AudioManager::getNewMidiId()
{
	uint32_t size = (uint32_t)_midiPlayers.size() + 1; // 0 is reserved for invalid id
	for (uint32_t i = 1; i < size; i++)
	{
		if (_midiPlayers.count(i) == 0)
			return i;
	}
	return size;
}


void AudioManager::addWavPlayer(const string& key, const DynamicArray<uint8_t>& wav)
{
	audioMixer.addWavBuffer(wav, key);
}
void AudioManager::addMidiPlayer(const string& key, const DynamicArray<uint8_t>& midi)
{
	if (instance->_midiDataCache.count(key) == 0)
		instance->_midiDataCache[key] = midi;
}

uint32_t AudioManager::playWav(const string& key, bool infinite, int volume)
{
	if (key.empty()) return INVALID_ID;

	// TODO: I don't realy save the new id in _audioPlayers, so audioMixer.playWav returns the same number every time
	return audioMixer.playWav(key, volume / 100.f, infinite);
}
uint32_t AudioManager::playMidi(const string& key, bool infinite)
{
	if (key.empty()) return INVALID_ID;

	auto it = instance->_midiDataCache.find(key);
	if (it == instance->_midiDataCache.end())
	{
		LOG("[Warning] " __FUNCTION__ ": key not found. key: %s", key.c_str());
		return INVALID_ID;
	}

	uint32_t id = instance->getNewMidiId();
	instance->_midiPlayers[id] = make_shared<MidiPlayer>(key, it->second);
	instance->_midiPlayers[id]->play(infinite);
	return id;
}

void AudioManager::stopWav(uint32_t id)
{
	audioMixer.stop(id);
}
void AudioManager::removeWav(uint32_t id)
{
	audioMixer.remove(id);
}

void AudioManager::stopMidi(uint32_t id) {
	auto it = instance->_midiPlayers.find(id);
	if (it != instance->_midiPlayers.end())
		it->second->stop();
}
void AudioManager::removeMidi(uint32_t id) {
	auto it = instance->_midiPlayers.find(id);
	if (it != instance->_midiPlayers.end())
		instance->_midiPlayers.erase(it);
}

void AudioManager::remove(function<bool(const string& key)> predicate)
{
	for (auto it = instance->_midiPlayers.begin(); it != instance->_midiPlayers.end();)
	{
		if (predicate(it->second->getKey()))
			it = instance->_midiPlayers.erase(it);
		else
			++it;
	}

	for (auto it = instance->_midiDataCache.begin(); it != instance->_midiDataCache.end();)
	{
		if (predicate(it->first))
			it = instance->_midiDataCache.erase(it);
		else
			++it;
	}

	audioMixer.remove(predicate);
}

// returns the duration of the sound in milliseconds for WAV files
uint32_t AudioManager::getWavDuration(const string& key)
{
	return (uint32_t)audioMixer.getWavDuration(key);
}
void AudioManager::setVolume(uint32_t id, int volume)
{
	auto it = instance->_midiPlayers.find(id);
	if (it != instance->_midiPlayers.end())
		it->second->setVolume(volume);
}

