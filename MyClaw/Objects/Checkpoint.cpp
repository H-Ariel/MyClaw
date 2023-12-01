#include "Checkpoint.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../SavedGameManager.h"


Checkpoint::Checkpoint(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _state(States::Down),
	_imageSetPath(PathManager::getImageSetPath(obj.imageSet)), _superCheckpoint(0), _isSaved(false)
{
	_ani = AssetsManager::createAnimationFromPidImage(_imageSetPath + "/001.PID");
	setObjectRectangle();
	myMemCpy<int32_t>(ZCoord, DefaultZCoord::CheckpointFlag);

	if (obj.logic == "FirstSuperCheckpoint")
		_superCheckpoint = 1;
	else if (obj.logic == "SecondSuperCheckpoint") 
		_superCheckpoint = 2;
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
			_state = States::Rise;
			player->startPosition = position;

			if (_superCheckpoint)
			{
				// TODO: save player state (only in the first time)
				SavedGameManager::GameData data = {}; // player->getData();
				data.savePoint = (SavedGameManager::SavePoints)_superCheckpoint;
				SavedGameManager::save(data);
			}
		}
		break;

	case States::Rise:
		_ani->Logic(elapsedTime);
		if (_ani->isFinishAnimation())
		{
			_ani = AssetsManager::loadAnimation("GAME/ANIS/CHECKPOINT/WAVE.ANI", _imageSetPath);
			_state = States::Wave;
		}
		break;
	}
}
