#include "AudioManager.h"
#include "MidiPlayer.h"
#include "WavPlayer.h"


AudioManager* AudioManager::instance = nullptr;


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


AudioManager::AudioManager() : running(true) {
	//	AudioManager::playQueue();

	// open threads-pool to play the sounds from the queue
	for (int i = 0; i < NUM_OF_PLAYING_THREADS; i++)
		threads[i] = DBG_NEW thread(&AudioManager::playFromQueue, this);
}
AudioManager::~AudioManager() {
	running = false;

	audioQueueMutex.lock();
	while (!audioQueue.empty())
		audioQueue.pop();
	audioQueueMutex.unlock();

	for (int i = 0; i < NUM_OF_PLAYING_THREADS; i++)
	{
		threads[i]->join();
		delete threads[i];
	}
}

uint32_t AudioManager::getNewId()
{
	uint32_t size = (uint32_t)_audioPlayers.size() + 1; // 0 is reserved for invalid id
	for (uint32_t i = 1; i < size; i++)
	{
		if (_audioPlayers.count(i) == 0)
			return i;
	}
	return size;
}


void AudioManager::addWavPlayer(const string& key_, shared_ptr<BufferReader> wavReader)
{
	string key = key_;
	if (key[0] == '/')
		key = key.substr(1); // remove the first character

	if (instance->_audioDataCache.count(key) == 0)
	{
		WAVEFORMATEX fmt = {};
		uint32_t soundDataLength = 0;
		vector<uint8_t> wavSoundData;

		fmt.cbSize = sizeof(WAVEFORMATEX);
		wavReader->skip(20);
		wavReader->read(fmt.wFormatTag);
		wavReader->read(fmt.nChannels);
		wavReader->read(fmt.nSamplesPerSec);
		wavReader->read(fmt.nAvgBytesPerSec);
		wavReader->read(fmt.nBlockAlign);
		wavReader->read(fmt.wBitsPerSample);
		wavReader->skip(4);
		wavReader->read(soundDataLength);
		wavSoundData = wavReader->ReadVector(soundDataLength, true);

		soundDataLength = min(soundDataLength, (uint32_t)wavSoundData.size());

		instance->_audioDataCache[key] = { fmt, wavSoundData };
	}
}
void AudioManager::addMidiPlayer(const string& key, const vector<uint8_t>& midi)
{
	if (instance->_audioDataCache.count(key) == 0)
		instance->_audioDataCache[key] = { {}, midi };
}

uint32_t AudioManager::playWav(const string& key, bool infinite)
{
	if (key.empty()) return INVALID_ID;

	auto it = instance->_audioDataCache.find(key);
	if (it == instance->_audioDataCache.end())
	{
		LogFile::log(LogFile::Warning, "AudioManager::playMidi: key not found. key: %s", key.c_str());
		return INVALID_ID;
	}

	uint32_t id = instance->getNewId();

	auto& [fmt, wavSoundData] = it->second;
	instance->_audioPlayers[id] = make_shared<WavPlayer>(key, fmt, wavSoundData);
	instance->audioQueueMutex.lock();
	instance->audioQueue.push({ instance->_audioPlayers[id], infinite });
	instance->audioQueueMutex.unlock();

	return id;
}
uint32_t AudioManager::playMidi(const string& key, bool infinite)
{
	if (key.empty()) return INVALID_ID;

	auto it = instance->_audioDataCache.find(key);
	if (it == instance->_audioDataCache.end())
	{
		LogFile::log(LogFile::Warning, "AudioManager::playMidi: key not found. key: %s", key.c_str());
		return INVALID_ID;
	}

	uint32_t id = instance->getNewId();

	auto& [fmt, midiData] = it->second;
	instance->_audioPlayers[id] = make_shared<MidiPlayer>(key, midiData);
	instance->audioQueueMutex.lock();
	instance->audioQueue.push({ instance->_audioPlayers[id], infinite });
	instance->audioQueueMutex.unlock();

	return id;
}

void AudioManager::stop(uint32_t id)
{
	auto it = instance->_audioPlayers.find(id);
	if (it != instance->_audioPlayers.end())
		it->second->stop();
}
void AudioManager::remove(uint32_t id)
{
	auto it = instance->_audioPlayers.find(id);
	if (it != instance->_audioPlayers.end())
		instance->_audioPlayers.erase(it);
}
void AudioManager::remove(function<bool(const string& key)> predicate)
{
	for (auto it = instance->_audioPlayers.begin(); it != instance->_audioPlayers.end();)
	{
		if (predicate(it->second->getKey()))
			it = instance->_audioPlayers.erase(it);
		else
			++it;
	}

	for (auto it = instance->_audioDataCache.begin(); it != instance->_audioDataCache.end();)
	{
		if (predicate(it->first))
			it = instance->_audioDataCache.erase(it);
		else
			++it;
	}
}

// returns the duration of the sound in milliseconds for WAV files
uint32_t AudioManager::getDuration(const string& key)
{
	const auto& [fmt, wavSoundData] = instance->_audioDataCache[key];
	uint32_t d = (uint32_t)((float)wavSoundData.size() / fmt.nAvgBytesPerSec * 1000); // duartion in milliseconds
	return d ? d : 1;
}
void AudioManager::setVolume(uint32_t id, int volume)
{
	auto it = instance->_audioPlayers.find(id);
	if (it != instance->_audioPlayers.end())
		it->second->setVolume(volume);
}

void AudioManager::playFromQueue() {
	shared_ptr<IAudioPlayer> player;
	bool inf = false;
	bool play = false;
	while (running)
	{
		play = false;

		audioQueueMutex.lock();
		if (!audioQueue.empty())
		{
			player = audioQueue.front().first;
			inf = audioQueue.front().second;
			audioQueue.pop();
			play = true;
		}
		audioQueueMutex.unlock();

		if (!play)
			this_thread::sleep_for(chrono::milliseconds(100));
		else
		{
			if (isinstance<MidiPlayer>(player.get()))
				player->play(inf); // in case of MIDI sound, we don't need to open new thread to play it because it's already async
			else if (inf)
				thread([&]() { player->play(inf); }).detach(); // in case of infinite sound, open new thread to play it (TODO: find when we realy use this)
			else
			{
				player->play(inf);
				while (running && player->isPlaying())
					this_thread::sleep_for(chrono::milliseconds(100));
			}
		}
	}
}
