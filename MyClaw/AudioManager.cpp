#include "AudioManager.h"


// TODO: do not use this mutex
// TODO: if used: move to `AudioManager` (without `static` keyword)
static mutex mtx;

// TODO: `_wavPlayers` store only the player (without path) So that we can play the same file several times at the same time (like MagicClaw)

AudioManager::AudioManager(RezArchive* rezArchive)
	: _rezArchive(rezArchive), _currBgMusicType(BackgroundMusicType::None)
{
}
AudioManager::~AudioManager() // todo: make sure we need this function...
{
	for (auto& w : _wavPlayers)
		w.second->stop();
	
	if (_currBgMusic)
		_currBgMusic->stop();
}

shared_ptr<WavPlayer> AudioManager::getWavPlayer(string wavFilePath)
{
	if (_wavPlayers.count(wavFilePath) == 0)
	{
		_wavPlayers[wavFilePath].reset(DBG_NEW WavPlayer(_rezArchive->getFileBufferReader(wavFilePath)));
	}
	return _wavPlayers[wavFilePath];
}

void AudioManager::playWavFile(string wavFilePath, bool infinite)
{
	if (wavFilePath.empty()) return;
	mtx.lock();
	getWavPlayer(wavFilePath)->play(infinite);
	mtx.unlock();
}
void AudioManager::stopWavFile(string wavFilePath)
{
	if (wavFilePath.empty()) return;
	mtx.lock();
	getWavPlayer(wavFilePath)->stop();
	mtx.unlock();
}

uint32_t AudioManager::getWavFileDuration(string wavFilePath)
{
	if (wavFilePath.empty()) return 0;
	mtx.lock();
	uint32_t d = getWavPlayer(wavFilePath)->getDuration();
	mtx.unlock();
	return d;
}

void AudioManager::setVolume(string wavFilePath, int32_t volume)
{
	if (wavFilePath.empty()) return;
	mtx.lock();
	getWavPlayer(wavFilePath)->setVolume(volume);
	mtx.unlock();
}

void AudioManager::clearLevelSounds(string prefix)
{
	mtx.lock();
	vector<string> keysToRemove;
	for (auto& a : _wavPlayers)
	{
		if (startsWith(a.first, prefix))
		{
			keysToRemove.push_back(a.first);
		}
	}
	for (const string& k : keysToRemove)
	{
		_wavPlayers.erase(k);
	}

	_midiPlayers.clear();
	_currBgMusic = nullptr;
	_currBgMusicType = BackgroundMusicType::None;
	mtx.unlock();
}

void AudioManager::checkForRestart()
{
	vector<string> keysToReplay;
	for (auto& w : _wavPlayers)
	{
		mtx.lock();
		if (w.second->shouldPlay())
		{
			keysToReplay.push_back(w.first);
		}
		mtx.unlock();
	}
	for (const string& k : keysToReplay)
	{
		playWavFile(k, _wavPlayers[k]->isInfinite());
	}
}

void AudioManager::setBackgroundMusic(BackgroundMusicType type, bool reset)
{
	static mutex myMutex;
	myMutex.lock();
	if (type != _currBgMusicType)
	{
		if (_midiPlayers.count(type) == 0)
		{
			_midiPlayers[BackgroundMusicType::Level] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData(PathManager::getBackgroundMusicFilePath("LEVEL_PLAY")));
			_midiPlayers[BackgroundMusicType::Powerup] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData(PathManager::getBackgroundMusicFilePath("GAME_POWERUP")));
			try {
				_midiPlayers[BackgroundMusicType::Boss] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData(PathManager::getBackgroundMusicFilePath("LEVEL_BOSS")));
			}
			catch (Exception&) {}
		}

		_currBgMusicType = type;

		if (_currBgMusic != nullptr)
			_currBgMusic->stop();
		_currBgMusic = _midiPlayers[type];
		if (reset)
			_currBgMusic->reset();

		if (!_currBgMusic->isPlaying())
			_currBgMusic->play(true);

		// TODO: if we continue the music (e.g. after powerup end) sometimes it play wrong
	}
	myMutex.unlock();
}
