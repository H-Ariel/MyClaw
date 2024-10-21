#include "AudioManager.h"
#include "MidiPlayer.h"


AudioManager* AudioManager::instance = nullptr;


static int clamp(int value, int min, int max) {
	return value < min ? min : value > max ? max : value;
}

#ifdef _DEBUG
// writes the error message in the debuggers output window
static void WAV_CALL(MMRESULT mmResult)
{
	if (mmResult != MMSYSERR_NOERROR)
	{
		char text[256];
		waveOutGetErrorTextA(mmResult, text, sizeof(text));
		LOG("[Error] WAV Error: %s\n", text);
	}
}
#else
#define WAV_CALL(func) func
#endif

#include "WindowManager.h"

#include <dsound.h>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

// Audio Mixer fo WAV files using WinApi
// TODO: combine with AudioManager to 1 class
class AudioMixer
{
private:
	struct AudioData;

public:
	AudioMixer()
		: volume(1), audioFormat({}), running(false), t_play(nullptr),
		g_pDirectSound(nullptr), g_pPrimaryBuffer(nullptr)
	{
		// set audio format:
		WAVEFORMATEX waveFormat = {};
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = 1;
		waveFormat.nSamplesPerSec = 44100;
		waveFormat.wBitsPerSample = 16;
		waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		waveFormat.cbSize = 0;
		myMemCpy(audioFormat, waveFormat);

		if (!InitializeDirectSound(WindowManager::getHwnd())) {
			LOG("[ERROR] Failed to initialize DirectSound\n");
			return;
		}

		if (g_pDirectSound) {
			running = true;
			t_play = DBG_NEW thread(&AudioMixer::playAudioThread, this);
		}
	}
	~AudioMixer() {
		clear();

		if (g_pPrimaryBuffer) {
			g_pPrimaryBuffer->Release();
			g_pPrimaryBuffer = nullptr;
		}

		if (g_pDirectSound) {
			g_pDirectSound->Release();
			g_pDirectSound = nullptr;
		}

		if (running) {
			running = false;
			t_play->join();
			delete t_play;
		}
	}

	// clear all audios
	void clear() {
		for (auto& it : savedAudios)
			clear(it.second);
		savedAudios.clear();
	}

	// if success returns `id`, returns INVALID_ID.
	uint32_t playWav(const string& key, float volume, bool infinite) {
		auto it = savedAudios.find(key);
		if (it == savedAudios.end())
		{
			LOG("[Warning] " __FUNCTION__ ": key not found. key: %s\n", key.c_str());
			return AudioManager::INVALID_ID;
		}

		AudioData audio = it->second;
		audio.volume = volume;
		audio.id = 20; // TODO: set real id
		audio.infinite = infinite;
		lock_guard<mutex> guard(audiosMutex);
		audios.push_back(audio);
		return audio.id;
	}

	void addWavBuffer(shared_ptr<Buffer> wavReader, const string& key) {
		if (savedAudios.find(key) != savedAudios.end()) return; // already exists

		WAVEFORMATEX fmt = {};

		wavReader->skip(20);
		wavReader->read(fmt.wFormatTag);
		wavReader->read(fmt.nChannels);
		wavReader->read(fmt.nSamplesPerSec);
		wavReader->read(fmt.nAvgBytesPerSec);
		wavReader->read(fmt.nBlockAlign);
		wavReader->read(fmt.wBitsPerSample);
		wavReader->skip(4);
		DynamicArray<uint8_t> wavSoundData = wavReader->readBytes(wavReader->read<uint32_t>(), true); // read length and then read bytes
		DynamicArray<int16_t> convertedData = matchFormat(fmt, wavSoundData);
		if (convertedData.size() == 0) return;

		// update fields
		fmt.nBlockAlign = fmt.nChannels * (fmt.wBitsPerSample / 8);
		fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

		// check that all wav files in the same format:
		if (memcmp(&fmt, &audioFormat, sizeof(WAVEFORMATEX)) != 0) {
			LOG("[ERROR] formats are not equals\n");
			return;
		}

		// save new audio
		AudioData audioData;
		audioData.soundBufferLength = (int)convertedData.size();
		audioData.soundBuffer = DBG_NEW int16_t[audioData.soundBufferLength];
		memcpy(audioData.soundBuffer, convertedData.data(), convertedData.size() * sizeof(convertedData[0]));
		audioData.key = key;
		audioData.reset(); // init currPtr and currLength
		savedAudios[key] = audioData;
	}

	// duartion in milliseconds
	float getWavDuration(const string& key) {
		return (float)savedAudios[key].soundBufferLength / audioFormat.nAvgBytesPerSec * 1000;
	}

	// TODO: make sure i know what I want to do with `stop` and `remove`... :/
	void stop(uint32_t id) { remove(id); }

	void remove(uint32_t id) {
		audios.remove_if([&](const AudioData& a) { return a.id == id; });
	}

	void remove(function<bool(const string& key)> predicate) {
		lock_guard<mutex> guard(audiosMutex);

		for (auto it = savedAudios.begin(); it != savedAudios.end();)
		{
			if (predicate(it->first)) {
				audios.remove_if([&](const AudioData& a) { return a.soundBuffer == it->second.soundBuffer; }); // remove the buffer we freed
				clear(it->second);
				it = savedAudios.erase(it);
			}
			else
				++it;
		}
	}

private:
	bool InitializeDirectSound(HWND hwnd) {
		if (FAILED(DirectSoundCreate8(nullptr, &g_pDirectSound, nullptr))) {
			return false;
		}

		if (FAILED(g_pDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))) {
			return false;
		}

		DSBUFFERDESC bufferDesc = {};
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
		bufferDesc.dwBufferBytes = 0;
		bufferDesc.lpwfxFormat = nullptr;

		if (FAILED(g_pDirectSound->CreateSoundBuffer(&bufferDesc, &g_pPrimaryBuffer, nullptr))) {
			return false;
		}

		if (FAILED(g_pPrimaryBuffer->SetFormat(&audioFormat))) {
			return false;
		}

		return true;
	}

	void playAudioThread() {
		int streamLen = 16384;
		int16_t* stream = nullptr;
		int retLen = 0;
		stream = DBG_NEW int16_t[streamLen];
		DWORD dwStatus = 0;

		void* pLockedBuffer = nullptr;
		DWORD bufferSize = 0;

		DSBUFFERDESC bufferDesc = {};
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.lpwfxFormat = (LPWAVEFORMATEX)&audioFormat;

		IDirectSoundBuffer* g_pSecondaryBuffer = nullptr;

		while (running) {
			// mix next buffer while previous is playing
			retLen = mixAudios(stream, streamLen);

			if (retLen > 0) {
				if (g_pSecondaryBuffer) {
					// wait until the buffer has finished playing
					do g_pSecondaryBuffer->GetStatus(&dwStatus);
					while (dwStatus & DSBSTATUS_PLAYING);

					g_pSecondaryBuffer->Release();
					g_pSecondaryBuffer = nullptr;

					if (!running) return;
				}

				bufferDesc.dwBufferBytes = retLen * sizeof(int16_t);

				if (FAILED(g_pDirectSound->CreateSoundBuffer(&bufferDesc, &g_pSecondaryBuffer, nullptr)))
					continue;

				if (FAILED(g_pSecondaryBuffer->Lock(0, bufferDesc.dwBufferBytes, &pLockedBuffer, &bufferSize, nullptr, nullptr, 0)))
					continue;

				memcpy(pLockedBuffer, stream, bufferSize); // usually `bufferSize` equals to `bufferDesc.dwBufferBytes`, so I do not compare them
				g_pSecondaryBuffer->Unlock(pLockedBuffer, bufferSize, nullptr, 0);

				g_pSecondaryBuffer->SetCurrentPosition(0);
				g_pSecondaryBuffer->Play(0, 0, 0);
			}
		}

		delete[] stream;
	}


	// mixes all active audio tracks into output stream and returns the number of samples written
	int mixAudios(int16_t* stream, int len) {
		if (audios.empty()) return 0;

		int* sum = DBG_NEW int[len]; // sum of voices in each index
		for (int i = 0; i < len; sum[i++] = 0);

		int writtenLen = 0;

		lock_guard<mutex> guard(audiosMutex);
		for (auto it = audios.begin(); it != audios.end(); ) {
			auto& audio = *it;
			if (audio.currLength <= 0) {
				if (audio.infinite)
					audio.reset();
				else {
					it = audios.erase(it); // remove finished audio from list
					continue;
				}
			}

			int lengthToMix = min(len, audio.currLength);

			for (int j = 0; j < lengthToMix; j++)
				sum[j] += (int)(audio.currPtr[j] * audio.volume); // apply volume scaling

			audio.currPtr += lengthToMix;
			audio.currLength -= lengthToMix;

			if (writtenLen < lengthToMix)
				writtenLen = lengthToMix;

			++it;
		}

		for (int i = 0; i < len; i++) {
			int val = (int)(sum[i] * volume);
			stream[i] = (int16_t)clamp(val, INT16_MIN, INT16_MAX); // Clamp to 16-bit signed range
		}

		delete[] sum;

		return writtenLen;
	}

	// converts WAV sound data to 16-bit format and resamples it to match the target sample rate (44100 KHz)
	DynamicArray<int16_t> matchFormat(WAVEFORMATEX& fmt, const DynamicArray<uint8_t>& wavSoundData) const {
		// convert to 16-bit format
		DynamicArray<int16_t> convertedData;
		if (fmt.wBitsPerSample == 16) { // just change pointer type
			convertedData = DynamicArray<int16_t>(wavSoundData.size() / 2); // same size in bytes
			memcpy(convertedData.data(), wavSoundData.data(), wavSoundData.size());
		}
		else if (fmt.wBitsPerSample == 8) { // Extend
			convertedData = DynamicArray<int16_t>(wavSoundData.size());
			for (int i = 0; i < wavSoundData.size(); i++)
				convertedData[i] = static_cast<int16_t>(((int)wavSoundData[i] - 0x80) << 8);
			fmt.wBitsPerSample = 16; // update format
		}
		else {
			LOG("[ERROR] " __FUNCTION__ " format not supported\n");
			return {}; // return empty if format is unsupported
		}

		// resample if needed
		if (fmt.nSamplesPerSec != audioFormat.nSamplesPerSec) {
			double resampleRatio = (double)audioFormat.nSamplesPerSec / fmt.nSamplesPerSec;
			int newSampleCount = (int)(convertedData.size() * resampleRatio);
			DynamicArray<int16_t> resampledData(newSampleCount);
			for (int i = 0; i < newSampleCount; i++) {
				double originalIndex = i / resampleRatio;
				int index1 = (int)(originalIndex);
				int index2 = min(index1 + 1, (int)(convertedData.size()) - 1);
				double frac = originalIndex - index1;

				// linear interpolation
				resampledData[i] = clamp(
					(int)(convertedData[index1] * (1 - frac) + convertedData[index2] * frac),
					INT16_MIN, INT16_MAX);
			}
			fmt.nSamplesPerSec = audioFormat.nSamplesPerSec; // update sample rate
			return resampledData;
		}

		return convertedData; // return converted data
	}

	void clear(AudioData& audioData) {
		delete[] audioData.soundBuffer;
		//audioData.soundBuffer = nullptr;
		//audioData.soundBufferLength = 0;
		//audioData.reset();
	}

	struct AudioData {
		int16_t* soundBuffer;
		int soundBufferLength;
		int16_t* currPtr; // pointer to current data in `soundBuffer`
		int currLength;   // remains bytes (matches with currPtr)
		float volume;     // volume multiplier for all audios - 0.0 to 1.0
		uint32_t id;
		bool infinite;
		string key; // TODO just for debug


		AudioData()
			: soundBuffer(nullptr), soundBufferLength(0),
			currPtr(nullptr), currLength(0),
			volume(1), id(0), infinite(false) { }

		void reset() {
			currPtr = soundBuffer;
			currLength = soundBufferLength;
		}
	};

	map<string, AudioData> savedAudios; // [key]=wav
	list<AudioData> audios;
	mutex audiosMutex; // mutex for `list<> audios`
	float volume; // volume multiplier for all audios - 0.0 to 1.0
	thread* t_play;
	bool running;
	const WAVEFORMATEX audioFormat;

	IDirectSound8* g_pDirectSound;
	IDirectSoundBuffer* g_pPrimaryBuffer;
};

AudioMixer* audioMixer = nullptr;


void AudioManager::Initialize()
{
	if (instance == nullptr) {
		instance = DBG_NEW AudioManager();
		audioMixer = DBG_NEW AudioMixer();
	}
}
void AudioManager::Finalize()
{
	if (instance)
	{
		delete instance;
		delete audioMixer;
		instance = nullptr;
		audioMixer = nullptr;
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
	// TODO: get Buffer as parameter
	audioMixer->addWavBuffer(make_shared<Buffer>(wav), key);
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
	return audioMixer->playWav(key, volume / 100.f, infinite);
}
uint32_t AudioManager::playMidi(const string& key, bool infinite)
{
	if (key.empty()) return INVALID_ID;

	auto it = instance->_midiDataCache.find(key);
	if (it == instance->_midiDataCache.end())
	{
		LOG("[Warning] " __FUNCTION__ ": key not found. key: %s\n", key.c_str());
		return INVALID_ID;
	}

	uint32_t id = instance->getNewMidiId();
	instance->_midiPlayers[id] = make_shared<MidiPlayer>(key, it->second);
	instance->_midiPlayers[id]->play(infinite);
	return id;
}

void AudioManager::stopWav(uint32_t id)
{
	audioMixer->stop(id);
}
void AudioManager::removeWav(uint32_t id)
{
	audioMixer->remove(id);
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

	audioMixer->remove(predicate);
}

// returns the duration of the sound in milliseconds for WAV files
uint32_t AudioManager::getWavDuration(const string& key)
{
	return (uint32_t)audioMixer->getWavDuration(key);
}
void AudioManager::setVolume(uint32_t id, int volume)
{
	auto it = instance->_midiPlayers.find(id);
	if (it != instance->_midiPlayers.end())
		it->second->setVolume(volume);
}
