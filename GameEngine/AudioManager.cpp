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

// Audio Mixer fo WAV files using WinApi
// TODO: sometimes there is a small delay before I listen the sound
// TODO: combine with AudioManager to 1 class
class AudioMixer
{
private:
	struct AudioData;

public:
	AudioMixer()
		: volume(1), audioFormat({}), wavHdr({}), wavOut(nullptr),
		running(false), t_play(nullptr), hEvent(nullptr)
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

		WAV_CALL(waveOutOpen(&wavOut, WAVE_MAPPER, &audioFormat, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION));
		if (!wavOut) {
			LOG("[ERROR] Faild create wav-out\n");
			return;
		}

		WAV_CALL(waveOutPrepareHeader(wavOut, &wavHdr, sizeof(wavHdr)));

		if (wavOut) {
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			running = true;
			t_play = DBG_NEW thread(&AudioMixer::playAudioThread, this);
		}
	}
	~AudioMixer() {
		clear();

		if (wavOut)
		{
			running = false;
			t_play->join();
			delete t_play;

			CloseHandle(hEvent);

			WAV_CALL(waveOutReset(wavOut));
			WAV_CALL(waveOutUnprepareHeader(wavOut, &wavHdr, sizeof(wavHdr)));
			WAV_CALL(waveOutClose(wavOut));
		}

		wavOut = nullptr;
		hEvent = nullptr;
		wavHdr = {};
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
		audio.reset();
		audio.volume = volume;
		audio.id = 0; // TODO: set real id
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
		if (memcmp(&fmt, &audioFormat, sizeof WAVEFORMATEX) != 0) {
			LOG("[ERROR] formats are not equals\n");
			return;
		}

		// save new audio
		AudioData audioData;
		audioData.soundBufferLength = (int)convertedData.size();
		audioData.soundBuffer = DBG_NEW int16_t[audioData.soundBufferLength];
		memcpy(audioData.soundBuffer, convertedData.data(), convertedData.size() * sizeof(convertedData[0]));
		audioData.key = key;
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
	void playAudioThread() {
		int streamLen = 32768; // TODO: find perfect size so we will not have any lags
		int16_t* stream = DBG_NEW int16_t[streamLen];

		while (running) {
			if (audios.empty())
				continue;

			// mix audio
			int retLen = mixAudios(stream, streamLen);

			// play mixed audio
			wavHdr.lpData = (LPSTR)stream;
			wavHdr.dwBufferLength = (DWORD)(retLen * sizeof(*stream));
			WAV_CALL(waveOutWrite(wavOut, &wavHdr, sizeof(wavHdr)));

			// Wait for playback completion by waiting for the event to be signaled
			WaitForSingleObject(hEvent, INFINITE);
		}

		delete[] stream;
	}

	// mixes all active audio tracks into output stream and returns the number of samples written
	int mixAudios(int16_t* stream, int len) {
		int* sum = DBG_NEW int[len]; // sum of voices in each index
		for (int i = 0; i < len; sum[i++] = 0);

		int writtenLen = 0;

		lock_guard<mutex> guard(audiosMutex);
		for (auto it = audios.begin(); it != audios.end(); ) {
			auto& audio = *it;
			if (audio.currLength <= 0) {
				if (audio.infinite)
					audio.reset();
				else
					it = audios.erase(it);
			}
			else {
				int lengthToMix = min(len, audio.currLength);

				for (int j = 0; j < lengthToMix; j++)
					sum[j] += (int)(audio.currPtr[j] * audio.volume); // apply volume scaling

				audio.currPtr += lengthToMix;
				audio.currLength -= lengthToMix;

				writtenLen = max(writtenLen, lengthToMix);

				++it;
			}
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

	static void waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		if (dwInstance) {
			AudioMixer* mixer = (AudioMixer*)dwInstance;
			if (uMsg == WOM_DONE) {
				SetEvent(mixer->hEvent);
			}
		}
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
	WAVEHDR wavHdr;
	HWAVEOUT wavOut;
	HANDLE hEvent; // event to signal playback completion
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
	// TODO: get Buffer as parameter
	audioMixer.addWavBuffer(make_shared<Buffer>(wav), key);
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

