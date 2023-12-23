#include "SoundTrigger.h"
#include "../Player.h"


SoundTrigger::SoundTrigger(const WwdObject& obj)
	: BaseSoundObject(obj), _timesToPlay(0), _isClawDialog(startsWith(obj.logic, "ClawDialog"))
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
		if (player->GetRect().intersects(_objRc))
		{
			_isInCollision = true;
			if (_timesCounter == -1 || _timesCounter > 0)
			{
				_wavPlayerId = AssetsManager::playWavFile(_wavPath, _volume);

				if (_timesCounter != -1)
				{
					_timesCounter -= 1;
				}

				if (_isClawDialog)
				{
					player->activateDialog(AssetsManager::getWavFileDuration(_wavPlayerId));
				}
			}
		}
	}
	else
	{
		_isInCollision = player->GetRect().intersects(_objRc);
	}
}
void SoundTrigger::Reset()
{
	_timesCounter = _timesToPlay;
	_isInCollision = false;
}
