#include "SoundTrigger.h"
#include "../GlobalObjects.h"
#include "Player/Player.h"


SoundTrigger::SoundTrigger(const WwdObject& obj)
	: BaseSoundObject(obj, true), _timesToPlay(0), _isClawDialog(startsWith(obj.logic, "ClawDialog"))
{
	int timesToPlay = 0;

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
		if (GO::getPlayerRect().intersects(_objRc))
		{
			_isInCollision = true;
			if (_timesCounter == -1 || _timesCounter > 0)
			{
				AssetsManager::playWavFile(_wavPath, _volume);

				if (_timesCounter != -1)
				{
					_timesCounter -= 1;
				}

				if (_isClawDialog)
				{
					GO::player->activateDialog(AssetsManager::getWavFileDuration(_wavPath));
				}
			}
		}
	}
	else
	{
		_isInCollision = GO::getPlayerRect().intersects(_objRc);
	}
}
void SoundTrigger::Reset()
{
	_timesCounter = _timesToPlay;
	_isInCollision = false;
}
