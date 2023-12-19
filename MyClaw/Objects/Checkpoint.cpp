#include "Checkpoint.h"
#include "../ActionPlane.h"
#include "../Player.h"
#include "../SavedGameManager.h"
#include "../Assets-Managers/AssetsManager.h"


Checkpoint::Checkpoint(const WwdObject& obj, int levelNumber)
	: BaseStaticPlaneObject(obj), _state(States::Down), _levelNumber(levelNumber),
	_imageSetPath(PathManager::getImageSetPath(obj.imageSet)), _superCheckpoint(0), _isSaved(false)
{
	_ani = AssetsManager::createAnimationFromPidImage(_imageSetPath + "/001.PID");
	setObjectRectangle();
	drawZ = DefaultZCoord::CheckpointFlag;

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

			if (_superCheckpoint && !_isSaved)
			{
				SavedGameManager::GameData data = player->getGameData();
				data.level = _levelNumber;
				data.savePoint = (SavedGameManager::SavePoints)_superCheckpoint;
				SavedGameManager::save(data);
				_isSaved = true;
				ActionPlane::writeMessage("Your game has been saved");
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
