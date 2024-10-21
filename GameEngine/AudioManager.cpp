#include "AudioManager.h"
#include "MidiPlayer.h"
#include "WindowManager.h" // get window HWND for dsound

#define WAV_STREAM_LEN 16384 // buffer's size for mixing


AudioManager* AudioManager::instance = nullptr;


static int clamp(int value, int min, int max) {
	return value < min ? min : value > max ? max : value;
}


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


AudioManager::AudioManager()
	: wavGlobalVolume(1), midiGlobalVolume(1), running(false), t_play(nullptr), dSound(nullptr), primaryBuffer(nullptr)
{
	// set audio format:
	WavFormat = {};
	WavFormat.wFormatTag = WAVE_FORMAT_PCM;
	WavFormat.nChannels = 1;
	WavFormat.nSamplesPerSec = 44100;
	WavFormat.wBitsPerSample = 16;
	WavFormat.nBlockAlign = WavFormat.nChannels * (WavFormat.wBitsPerSample / 8);
	WavFormat.nAvgBytesPerSec = WavFormat.nSamplesPerSec * WavFormat.nBlockAlign;
	WavFormat.cbSize = 0;

	DSBUFFERDESC bufferDesc = {};
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.lpwfxFormat = nullptr;

	TRY_HRESULT(DirectSoundCreate8(nullptr, &dSound, nullptr), "DirectSoundCreate8 Failed\n");
	TRY_HRESULT(dSound->SetCooperativeLevel(WindowManager::getHwnd(), DSSCL_PRIORITY), "AudioManager SetCooperativeLevel Failed\n");
	TRY_HRESULT(dSound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, nullptr), "AudioManager CreateSoundBuffer Failed\n");
	TRY_HRESULT(primaryBuffer->SetFormat(&WavFormat), "AudioManager SetFormat Failed\n");

	running = true;
	t_play = DBG_NEW thread(&AudioManager::playWavAudioThread, this);
}
AudioManager::~AudioManager() {
	// clear all wavAudios
	wavAudiosListMutex.lock();
	wavAudios.clear();
	wavAudiosListMutex.unlock();

	for (auto& it : savedWavAudios)
		delete[] it.second.soundBuffer;
	savedWavAudios.clear();

	primaryBuffer->Release();
	dSound->Release();

	if (running) {
		running = false;
		t_play->join();
		delete t_play;
	}
}

void AudioManager::addWavPlayer(const string& key, const DynamicArray<uint8_t>& wavFile)
{
	//	audioMixer->addWavBuffer(make_shared<Buffer>(wav), key);

	if (instance->savedWavAudios.find(key) != instance->savedWavAudios.end()) return; // already exists

	Buffer wavReader(wavFile);

	WAVEFORMATEX fmt = {};
	wavReader.skip(20);
	wavReader.read(fmt.wFormatTag);
	wavReader.read(fmt.nChannels);
	wavReader.read(fmt.nSamplesPerSec);
	wavReader.read(fmt.nAvgBytesPerSec);
	wavReader.read(fmt.nBlockAlign);
	wavReader.read(fmt.wBitsPerSample);
	wavReader.skip(4);
	DynamicArray<uint8_t> wavSoundData = wavReader.readBytes(wavReader.read<uint32_t>(), true); // read length and then read bytes
	DynamicArray<int16_t> convertedData = instance->matchWavFormat(fmt, wavSoundData);
	if (convertedData.size() == 0) return;

	// update fields
	fmt.nBlockAlign = fmt.nChannels * (fmt.wBitsPerSample / 8);
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

	// check that all wav files in the same format:
	if (memcmp(&fmt, &instance->WavFormat, sizeof(WAVEFORMATEX)) != 0) {
		LOG("[ERROR] formats are not equals\n");
		return;
	}

	// save new audio
	WavAudioData audioData = {};
	audioData.soundBufferLength = (int)convertedData.size();
	audioData.soundBuffer = DBG_NEW int16_t[audioData.soundBufferLength];
	memcpy(audioData.soundBuffer, convertedData.data(), convertedData.size() * sizeof(convertedData[0]));
	audioData.reset();
	instance->savedWavAudios[key] = audioData;
}
void AudioManager::addMidiPlayer(const string& key, const DynamicArray<uint8_t>& midi)
{
	if (instance->_midiDataCache.count(key) == 0)
		instance->_midiDataCache[key] = midi;
}

uint32_t AudioManager::playWav(const string& key, bool infinite, float volume)
{
	if (key.empty()) return INVALID_ID;

	auto it = instance->savedWavAudios.find(key);
	if (it == instance->savedWavAudios.end()) {
		LOG("[Warning] wav-key not found. key: %s\n", key.c_str());
		return INVALID_ID;
	}

	WavAudioData audio = it->second;
	audio.volume = volume;
	audio.infinite = infinite;
	lock_guard<mutex> guard(instance->wavAudiosListMutex);
	audio.id = instance->getNewWavId();
	instance->wavAudios.push_back(audio);
	return audio.id;

}
uint32_t AudioManager::playMidi(const string& key, bool infinite)
{
	if (key.empty()) return INVALID_ID;

	auto it = instance->_midiDataCache.find(key);
	if (it == instance->_midiDataCache.end())
	{
		LOG("[Warning] midi-key not found. key: %s\n", key.c_str());
		return INVALID_ID;
	}

	uint32_t id = instance->getNewMidiId();
	instance->_midiPlayers[id] = make_shared<MidiPlayer>(key, it->second);
	instance->_midiPlayers[id]->play(infinite);
	instance->_midiPlayers[id]->setVolume(instance->midiGlobalVolume);
	return id;
}

void AudioManager::stopWav(uint32_t id)
{
	instance->wavAudios.remove_if([&](const WavAudioData& a) { return a.id == id; });
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

void AudioManager::setWavVolume(float volume)
{
	instance->wavGlobalVolume = volume;
}
void AudioManager::setMidiVolume(float volume)
{
	instance->midiGlobalVolume = volume;
	MidiPlayer::setVolume(volume);
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

	lock_guard<mutex> guard(instance->wavAudiosListMutex);

	for (auto it = instance->savedWavAudios.begin(); it != instance->savedWavAudios.end();)
	{
		if (predicate(it->first)) {
			instance->wavAudios.remove_if([&](const WavAudioData& a) { return a.soundBuffer == it->second.soundBuffer; });
			delete[] it->second.soundBuffer;
			it = instance->savedWavAudios.erase(it);
		}
		else
			++it;
	}
}

uint32_t AudioManager::getWavDuration(const string& key)
{
	return (uint32_t)((float)instance->savedWavAudios[key].soundBufferLength / instance->WavFormat.nAvgBytesPerSec * 1000);
}


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
uint32_t AudioManager::getNewWavId()
{
	uint32_t i = INVALID_ID + 1; // 0 is reserved for invalid id
	bool found = false;
	while (!found) {
		found = true;
		for (const auto& a : wavAudios)
			if (a.id == i) {
				i++;
				found = false;
				break;
			}
	}
	return i;
}

// converts WAV sound data to 16-bit format and resamples it to match the target sample rate (44100 KHz)
DynamicArray<int16_t> AudioManager::matchWavFormat(WAVEFORMATEX& fmt, const DynamicArray<uint8_t>& wavSoundData) const {
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
		LOG("[ERROR] wav-format not supported\n");
		return {}; // return empty if format is unsupported
	}

	// resample if needed
	if (fmt.nSamplesPerSec != WavFormat.nSamplesPerSec) {
		double resampleRatio = (double)WavFormat.nSamplesPerSec / fmt.nSamplesPerSec;
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
		fmt.nSamplesPerSec = WavFormat.nSamplesPerSec; // update sample rate
		return resampledData;
	}

	return convertedData; // return converted data
}

// mixes all active audio tracks into output stream and returns the number of samples written
int AudioManager::mixWavAudios(int16_t* stream, int len) {
	if (wavAudios.empty()) return 0;

	int* sum = DBG_NEW int[len] {}; // sum of voices in each index

	int writtenLen = 0;

	wavAudiosListMutex.lock();
	for (auto it = wavAudios.begin(); it != wavAudios.end(); ) {
		auto& audio = *it;
		if (audio.currLength <= 0) {
			if (audio.infinite)
				audio.reset();
			else {
				it = wavAudios.erase(it); // remove finished audio from list
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
	wavAudiosListMutex.unlock();

	for (int i = 0; i < len; i++)
		stream[i] = (int16_t)clamp((int)(sum[i] * wavGlobalVolume), INT16_MIN, INT16_MAX); // Clamp to 16-bit signed range

	delete[] sum;

	return writtenLen;
}

void AudioManager::playWavAudioThread() {
	int16_t* stream = DBG_NEW int16_t[WAV_STREAM_LEN];
	int retLen;
	void* pLockedBuffer; // buffer from `Lock`
	DWORD bufferSize; // buffer size from `Lock`
	DWORD status;
	int durationMs;

	IDirectSoundBuffer* secondaryBuffer = nullptr;

	DSBUFFERDESC bufferDesc = {}; // buffer description for secondaryBuffer
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.lpwfxFormat = &WavFormat;


	while (running) {
		// mix next buffer (even while previous is playing)
		retLen = mixWavAudios(stream, WAV_STREAM_LEN);

		if (retLen > 0) {
			if (secondaryBuffer) {
				// wait until the buffer has finished playing
				do secondaryBuffer->GetStatus(&status);
				while ((status & DSBSTATUS_PLAYING) && running);

				secondaryBuffer->Release();
				secondaryBuffer = nullptr;
			}

			if (!running)
				break;

			bufferDesc.dwBufferBytes = retLen * sizeof(int16_t);

			if (FAILED(dSound->CreateSoundBuffer(&bufferDesc, &secondaryBuffer, nullptr)))
				continue;
			if (FAILED(secondaryBuffer->Lock(0, bufferDesc.dwBufferBytes, &pLockedBuffer, &bufferSize, nullptr, nullptr, 0)))
				continue;

			memcpy(pLockedBuffer, stream, bufferSize); // usually `bufferSize` equals to `bufferDesc.dwBufferBytes`, so I do not compare them
			secondaryBuffer->Unlock(pLockedBuffer, bufferSize, nullptr, 0);

			secondaryBuffer->SetCurrentPosition(0);
			secondaryBuffer->Play(0, 0, 0);

			// calculate duration in milliseconds
			durationMs = int(float(retLen) / instance->WavFormat.nSamplesPerSec * 1000);
			std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
		}
	}

	delete[] stream;
	if (secondaryBuffer)
		secondaryBuffer->Release();
}

