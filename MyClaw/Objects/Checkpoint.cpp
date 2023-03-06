#include "Checkpoint.h"
#include "../Player.h"
#include "../AssetsManager.h"


Checkpoint::Checkpoint(const WwdObject& obj, Player* player)
	: Checkpoint(obj, player, "GAME/IMAGES/CHECKPOINTFLAG/")
{
}
Checkpoint::Checkpoint(const WwdObject& obj, Player* player, string imageSetPath)
	: BaseStaticPlaneObject(obj, player), _state(States::Down), _imageSetPath(imageSetPath)
{
	/*string imageName;
	if (_imageSetPath == "GAME/IMAGES/SUPERCHECKPOINT/")
	{
		imageName = "FRAME";
	}
	_ani = AssetsManager::createCopyAnimationFromFromPidImage(_imageSetPath + imageName + "01.PID");*/
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

SuperCheckpoint::SuperCheckpoint(const WwdObject& obj, Player* player)
	: Checkpoint(obj, player, "GAME/IMAGES/SUPERCHECKPOINT/")
{
}
void SuperCheckpoint::Logic(uint32_t elapsedTime)
{
	if (CollisionDistances::isCollision(_objRc, _player->GetRect()))
	{
		// TODO: save player state (only in the first time)
	}
	Checkpoint::Logic(elapsedTime);
}
