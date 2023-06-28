#include "AudioManager.h"


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

uint32_t AudioManager::getNewId()
{
	for (uint32_t i = 0; i < _wavPlayers.size(); i++)
	{
		if (_wavPlayers.count(i) == 0)
			return i;
	}
	return (uint32_t)_wavPlayers.size();
}


uint32_t AudioManager::playWavFile(string wavFilePath, bool infinite)
{
	if (wavFilePath.empty()) return -1;

	uint32_t id = getNewId();

	_wavPlayers[id] = allocNewSharedPtr<WavPlayer>(_rezArchive->getFileBufferReader(wavFilePath));
	_wavPlayers[id]->play(infinite);

	return id;
}
void AudioManager::stopWavFile(uint32_t wavFileId)
{
	if (_wavPlayers.count(wavFileId) != 0)
		_wavPlayers.erase(wavFileId);
}

uint32_t AudioManager::getWavFileDuration(uint32_t wavFileId)
{
	uint32_t d = 0;
	
	if (_wavPlayers.count(wavFileId) != 0)
		d = _wavPlayers[wavFileId]->getDuration();
	
	return d ? d : 1;
}

void AudioManager::setVolume(uint32_t wavFileId, int32_t volume)
{
	if (_wavPlayers.count(wavFileId) != 0)
		_wavPlayers[wavFileId]->setVolume(volume);
}

void AudioManager::clearLevelSounds(string prefix)
{
	_wavPlayers.clear();
	_midiPlayers.clear();
	_currBgMusic = nullptr;
	_currBgMusicType = BackgroundMusicType::None;
}

void AudioManager::checkForRestart()
{
	for (auto& w : _wavPlayers)
	{
		if (w.second->shouldPlay())
			w.second->play(w.second->isInfinite());
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
			catch (const Exception&) {}
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
