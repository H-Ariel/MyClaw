#include "AudioManager.h"
#include "MidiPlayer.h"


AudioManager::AudioManager(RezArchive* rezArchive)
	: _rezArchive(rezArchive), _currBgMusicType(BackgroundMusicType::None)
{
	_midiPlayers[BackgroundMusicType::Powerup] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData("GAME/MUSIC/POWERUP.XMI"));
	_midiPlayers[BackgroundMusicType::Credits] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData("STATES/CREDITS/MUSIC/PLAY.XMI"));
	_midiPlayers[BackgroundMusicType::Boss] = allocNewSharedPtr<MidiPlayer>(_rezArchive->getFileData("LEVEL2/MUSIC/BOSS.XMI"));
}

uint32_t AudioManager::playWavFile(const string& wavFilePath, bool infinite)
{
	if (wavFilePath.empty()) return -1;

	uint32_t id = getWavNewId();
	auto [fmt, wavSoundData, soundDataLength] = getWavData(wavFilePath);
	_wavPlayers[id] = allocNewSharedPtr<WavPlayer>(fmt, wavSoundData, soundDataLength);
	_wavPlayers[id]->play(infinite);

	return id;
}
void AudioManager::stopWavFile(uint32_t wavFileId)
{
	if (_wavPlayers.count(wavFileId))
		_wavPlayers.erase(wavFileId);
}
uint32_t AudioManager::getWavFileDuration(uint32_t wavFileId)
{
	uint32_t d = 0;
	if (_wavPlayers.count(wavFileId))
		d = _wavPlayers[wavFileId]->getDuration();
	return d ? d : 1;
}
void AudioManager::setVolume(uint32_t wavFileId, int volume)
{
	if (_wavPlayers.count(wavFileId))
		_wavPlayers[wavFileId]->setVolume(volume);
}

void AudioManager::clearLevelSounds()
{
	stopBackgroundMusic();
	_wavPlayers.clear();
	_midiPlayers.erase(BackgroundMusicType::Level);
	_wavDataCache.clear(); // TODO: clear only level sounds
}
void AudioManager::checkForRestart()
{
	for (auto& [id, wavPlayer] : _wavPlayers)
	{
		if (wavPlayer->shouldPlay())
			wavPlayer->play(wavPlayer->isInfinite());
	}
}

void AudioManager::setBackgroundMusic(BackgroundMusicType type)
{
	thread([&](BackgroundMusicType type) {
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

			if (_currBgMusic)
				_currBgMusic->stop();
			_currBgMusic = _midiPlayers[type];

			if (!_currBgMusic->isPlaying())
				_currBgMusic->play(true);
		}
		_bgMutex.unlock();
	}, type).detach();
}
void AudioManager::stopBackgroundMusic()
{
	_bgMutex.lock();

	if (_currBgMusic)
		_currBgMusic->stop();
	_currBgMusic = nullptr;
	_midiPlayers.erase(BackgroundMusicType::Level);
	_currBgMusicType = BackgroundMusicType::None;

	_bgMutex.unlock();
}

tuple<WAVEFORMATEX, vector<uint8_t>, uint32_t> AudioManager::getWavData(const string& wavPath)
{
	if (_wavDataCache.count(wavPath) == 0)
	{
		shared_ptr<BufferReader> wavReader = _rezArchive->getFileBufferReader(wavPath);

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

		_wavDataCache[wavPath] = { fmt, wavSoundData, soundDataLength };
	}

	return _wavDataCache[wavPath];
}
uint32_t AudioManager::getWavNewId()
{
	for (uint32_t i = 0; i < _wavPlayers.size(); i++)
	{
		if (_wavPlayers.count(i) == 0)
			return i;
	}
	return (uint32_t)_wavPlayers.size();
}
