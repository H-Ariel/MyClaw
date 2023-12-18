#include "AmbientSound.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"


AmbientSound::AmbientSound(const WwdObject& obj)
	: BaseSoundObject(obj), _isPlaying(false)
{
}
void AmbientSound::Logic(uint32_t elapsedTime)
{
	if (player->GetRect().intersects(_objRc))
	{
		if (!_isPlaying)
		{
			_wavPlayerId = AssetsManager::playWavFile(_wavPath, _volume);
			_isPlaying = true;
		}
	}
	else
	{
		if (_isPlaying)
		{
			AssetsManager::stopWavFile(_wavPlayerId);
			_wavPlayerId = -1;
			_isPlaying = false;
		}
	}
}

GlobalAmbientSound::GlobalAmbientSound(const WwdObject& obj, int levelNumber)
	: BaseSoundObject(obj), _minTimeOn(obj.moveRect.left), _maxTimeOn(obj.moveRect.top),
	_minTimeOff(obj.moveRect.right), _maxTimeOff(obj.moveRect.bottom),
	_isLooping(obj.moveRect.left == 0), _currentTime(0), _soundDurationMs(0)
{
	_timeOff = getRandomInt(_minTimeOff, _maxTimeOff);

	if (_isLooping)
	{
		if (levelNumber == 2 || levelNumber == 3 || levelNumber == 4)
			_volume /= 5; // because the sound is too loud

		_wavPlayerId = AssetsManager::playWavFile(_wavPath, _volume, true);
		_soundDurationMs = AssetsManager::getWavFileDuration(_wavPlayerId);
	}
}
void GlobalAmbientSound::Logic(uint32_t elapsedTime)
{
	if (_isLooping)
	{
		return;
	}

	_currentTime += elapsedTime;
	if (_currentTime >= _timeOff)
	{
		_wavPlayerId = AssetsManager::playWavFile(_wavPath, _volume);
		_soundDurationMs = AssetsManager::getWavFileDuration(_wavPlayerId);

		int timeOn = getRandomInt(_minTimeOn, _maxTimeOn);
		int soundLoops = timeOn / _soundDurationMs;

		_timeOff = getRandomInt(_minTimeOff, _maxTimeOff) + soundLoops * _soundDurationMs;

		_currentTime = 0;
	}
}
