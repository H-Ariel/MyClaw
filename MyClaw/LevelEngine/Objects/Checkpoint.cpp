#include "Checkpoint.h"
#include "../GlobalObjects.h"
#include "Player/Player.h"
#include "ActionPlaneMessage.h"


Checkpoint::Checkpoint(const WwdObject& obj, int levelNumber)
	: BaseStaticPlaneObject(obj), _state(States::Down), _levelNumber(levelNumber),
	_imageSetPath(PathManager::getImageSetPath(obj.imageSet)), _superCheckpoint(0), _isSaved(false)
{
	_ani = AssetsManager::getAnimationFromPidImage(_imageSetPath + "/001.PID");
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
		if (_objRc.intersects(GO::getPlayerRect()))
		{
			_ani = AssetsManager::loadCopyAnimation("GAME/ANIS/CHECKPOINT/RISE.ANI", _imageSetPath);
			_state = States::Rise;
			GO::player->startPosition = position;

			if (_superCheckpoint && !_isSaved)
			{
				SavedDataManager::GameData data = GO::player->getGameData();
				data.level = _levelNumber;
				data.savePoint = (SavedDataManager::SavePoints)_superCheckpoint;
				SavedDataManager::saveGame(data);
				_isSaved = true;
				GO::addObjectToActionPlane(DBG_NEW ActionPlaneMessage("Your game has been saved"));
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
