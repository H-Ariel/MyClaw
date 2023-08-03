#include "AudioManager.h"
#include "MidiPlayer.h"


AudioManager::AudioManager(RezArchive* rezArchive)
	: _rezArchive(rezArchive), _currBgMusicType(BackgroundMusicType::None)
{
	_midiPlayers[BackgroundMusicType::Powerup] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData("GAME/MUSIC/POWERUP.XMI"));
	_midiPlayers[BackgroundMusicType::Credits] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData("STATES/CREDITS/MUSIC/PLAY.XMI"));
	_midiPlayers[BackgroundMusicType::Boss] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData("LEVEL2/MUSIC/BOSS.XMI"));
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

void AudioManager::clearLevelSounds()
{
	stopBackgroundMusic();
	_wavPlayers.clear();
	_midiPlayers.erase(BackgroundMusicType::Level);
}

void AudioManager::checkForRestart()
{
	for (auto& w : _wavPlayers)
	{
		if (w.second->shouldPlay())
			w.second->play(w.second->isInfinite());
	}
}

void AudioManager::setBackgroundMusic(BackgroundMusicType type)
{
	_bgMutex.lock();
	if (type != _currBgMusicType)
	{
		if (type == BackgroundMusicType::Level)
		{
			try {
				_midiPlayers[BackgroundMusicType::Level] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData(PathManager::getBackgroundMusicFilePath("LEVEL_PLAY")));
			}
			catch (const Exception&) {
				_bgMutex.unlock();
				return;
			}
		}

		_currBgMusicType = type;

		if (_currBgMusic != nullptr)
			_currBgMusic->stop();
		_currBgMusic = _midiPlayers[type];

		if (!_currBgMusic->isPlaying())
			_currBgMusic->play(true);
	}
	_bgMutex.unlock();
}

void AudioManager::stopBackgroundMusic()
{
	_bgMutex.lock();

	if (_currBgMusic != nullptr)
		_currBgMusic->stop();
	_currBgMusic = nullptr;
	_midiPlayers.erase(BackgroundMusicType::Level);
	_currBgMusicType = BackgroundMusicType::None;

	_bgMutex.unlock();
}
