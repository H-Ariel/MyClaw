#include "Checkpoint.h"
#include "../Player.h"
#include "../AssetsManager.h"


Checkpoint::Checkpoint(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _state(States::Down),
	_imageSetPath(PathManager::getImageSetPath(obj.imageSet)), _isSuperCheckpoint(contains(obj.logic, "Super"))
{
	_ani = AssetsManager::createCopyAnimationFromFromPidImage(_imageSetPath + "/001.PID");
	setObjectRectangle();
}
void Checkpoint::Logic(uint32_t elapsedTime)
{
	switch (_state)
	{
	case States::Down:
		// check if Claw touch the flag
		if (_objRc.intersects(player->GetRect()))
		{
			_ani = AssetsManager::loadCopyAnimation("GAME/ANIS/CHECKPOINT/RISE.ANI", _imageSetPath);
			_ani->position = position;
			_state = States::Rise;
			player->startPosition = position;

			if (_isSuperCheckpoint)
			{
				// TODO: save player state (only in the first time)
			}
		}
		break;

	case States::Rise:
		if (_ani->isFinishAnimation())
		{
			_ani = AssetsManager::loadCopyAnimation("GAME/ANIS/CHECKPOINT/WAVE.ANI", _imageSetPath);
			_ani->position = position;
			_state = States::Wave;
		}
		break;
	}

	_ani->Logic(elapsedTime);
}
