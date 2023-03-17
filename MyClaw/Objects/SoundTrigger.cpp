#include "SoundTrigger.h"
#include "../AssetsManager.h"
#include "../Player.h"


SoundObjectBase::SoundObjectBase(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player),
	_wavPath(PathManager::getSoundFilePath(obj.animation))
{
	// These objects are invisible so no need to animate them
	//_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet));

	_volume = obj.damage;
	if (_volume == 0)
	{
		_volume = 100;
	}
	AssetsManager::setWavFileVolume(_wavPath, _volume);

	D2D1_RECT_F newRc = {};

	if (obj.minX != 0)
	{
		newRc.left = (float)obj.minX;
		newRc.top = (float)obj.minY;
		newRc.right = (float)obj.maxX;
		newRc.bottom = (float)obj.maxY;
	}
	else
	{
		D2D1_SIZE_F size = {};

		if (contains(obj.logic, "Tiny"))
		{
			size = { 32, 32 };
		}
		else if (contains(obj.logic, "Small"))
		{
			size = { 64, 64 };
		}
		else if (contains(obj.logic, "Big"))
		{
			size = { 256, 256 };
		}
		else if (contains(obj.logic, "Huge"))
		{
			size = { 512, 512 };
		}
		else if (contains(obj.logic, "Wide"))
		{
			size = { 200, 64 };
		}
		else if (contains(obj.logic, "Tall"))
		{
			size = { 64, 200 };
		}
		else
		{
			size = { 128, 128 };
		}

		newRc.left = position.x - size.width / 2;
		newRc.top = position.y - size.height / 2;
		newRc.right = newRc.left + size.width;
		newRc.bottom = newRc.top + size.height;
	}

	myMemCpy(_objRc, newRc);
}

SoundTrigger::SoundTrigger(const WwdObject& obj, Player* player)
	: SoundObjectBase(obj, player), _timesToPlay(0), _isClawDialog(startsWith(obj.logic, "ClawDialog"))
{
	int32_t timesToPlay = 0;

	if (obj.smarts == -1)
	{
		timesToPlay = -1;
	}
	else if (obj.smarts == 0)
	{
		timesToPlay = 1;
	}
	else
	{
		timesToPlay = obj.smarts;
	}

	myMemCpy(_timesToPlay, timesToPlay);

	Reset();
}
void SoundTrigger::Logic(uint32_t elapsedTime)
{
	if (!_isInCollision)
	{
		if (CollisionDistances::isCollision(_player->GetRect(), _objRc))
		{
			_isInCollision = true;
			if (_timesCounter == -1 || _timesCounter > 0)
			{
				// TODO: use `_volume`
				AssetsManager::playWavFile(_wavPath);

				if (_timesCounter != -1)
				{
					_timesCounter -= 1;
				}

				if (_isClawDialog)
				{
					_player->activateDialog(AssetsManager::getWavFileDuration(_wavPath));
				}
			}
		}
	}
	else
	{
		_isInCollision = CollisionDistances::isCollision(_player->GetRect(), _objRc);
	}
}
void SoundTrigger::Reset()
{
	_timesCounter = _timesToPlay;
	_isInCollision = false;
}

AmbientSound::AmbientSound(const WwdObject& obj, Player* player)
	: SoundObjectBase(obj, player), _isPlaying(false)
{
}
void AmbientSound::Logic(uint32_t elapsedTime)
{
	if (CollisionDistances::isCollision(_player->GetRect(), _objRc))
	{
		if (!_isPlaying)
		{
			AssetsManager::playWavFile(_wavPath);
			_isPlaying = true;
		}
	}
	else
	{
		if (_isPlaying)
		{
			AssetsManager::stopWavFile(_wavPath);
			_isPlaying = false;
		}
	}
}

GlobalAmbientSound::GlobalAmbientSound(const WwdObject& obj, Player* player)
	: SoundObjectBase(obj, player),
	_minTimeOn(obj.moveRect.left),
	_maxTimeOn(obj.moveRect.top),
	_minTimeOff(obj.moveRect.right),
	_maxTimeOff(obj.moveRect.bottom),
	_isLooping(obj.moveRect.left == 0),
	_currentTime(0)
{
	_timeOff = getRandomInt(_minTimeOff, _maxTimeOff);
	_soundDurationMs = AssetsManager::getWavFileDuration(_wavPath);

	if (_isLooping)
	{
		AssetsManager::playWavFile(_wavPath, true);
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
		int timeOn = getRandomInt(_minTimeOn, _maxTimeOn);
		int soundLoops = timeOn / _soundDurationMs;

		AssetsManager::playWavFile(_wavPath);

		_timeOff = getRandomInt(_minTimeOff, _maxTimeOff) + soundLoops * _soundDurationMs;

		_currentTime = 0;
	}
}
