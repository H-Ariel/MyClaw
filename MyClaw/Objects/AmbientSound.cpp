#include "AmbientSound.h"
#include "../GlobalObjects.h"


AmbientSound::AmbientSound(const WwdObject& obj)
	: BaseSoundObject(obj), _isPlaying(false), _wavPlayerId(AssetsManager::INVALID_AUDIOPLAYER_ID)
{
}
void AmbientSound::Logic(uint32_t elapsedTime)
{
	if (GO::getPlayerRect().intersects(_objRc))
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
		//	AssetsManager::stopWavFile(_wavPlayerId);
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

		AssetsManager::playWavFile(_wavPath, _volume, true);
		_soundDurationMs = AssetsManager::getWavFileDuration(_wavPath);
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
		AssetsManager::playWavFile(_wavPath, _volume);
		_soundDurationMs = AssetsManager::getWavFileDuration(_wavPath);
		_timeOff = getRandomInt(_minTimeOff, _maxTimeOff) + getRandomInt(_minTimeOn, _maxTimeOn);
		_currentTime = 0;
	}
}
