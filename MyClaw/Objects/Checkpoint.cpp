#include "Checkpoint.h"
#include "../Player.h"
#include "../AssetsManager.h"


Checkpoint* Checkpoint::createCheckpoint(const WwdObject& obj, Player* player, bool isSuperCheckpoint)
{
	if (isSuperCheckpoint) return DBG_NEW Checkpoint(obj, player, "GAME/IMAGES/SUPERCHECKPOINT/", true);
	return DBG_NEW Checkpoint(obj, player, "GAME/IMAGES/CHECKPOINTFLAG/", false);
}

Checkpoint::Checkpoint(const WwdObject& obj, Player* player, string imageSetPath, bool isSuperCheckpoint)
	: BaseStaticPlaneObject(obj, player), _state(States::Down),
	_imageSetPath(imageSetPath), _isSuperCheckpoint(isSuperCheckpoint)
{
	_ani = AssetsManager::createCopyAnimationFromFromPidImage(_imageSetPath + "001.PID");
	setObjectRectangle();
}
void Checkpoint::Logic(uint32_t elapsedTime)
{
	switch (_state)
	{
	case States::Down:
		// check if Claw touch the flag
		if (CollisionDistances::isCollision(_objRc, _player->GetRect()))
		{
			_ani = AssetsManager::loadCopyAnimation("GAME/ANIS/CHECKPOINT/RISE.ANI", _imageSetPath);
			_ani->position = position;
			_state = States::Rise;
			_player->startPosition = position;

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
