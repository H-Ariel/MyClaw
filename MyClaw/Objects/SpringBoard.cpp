#include "SpringBoard.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"


SpringBoard::SpringBoard(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _force(sqrt(2 * GRAVITY * (obj.maxY > 0 ? obj.maxY : 450)))
{
	string anisPath;
	if (obj.imageSet == "LEVEL_WATERROCK") // level 7,13
	{
		string lvlNumber = PathManager::getImageSetPath("LEVEL_");

		if (lvlNumber == "LEVEL13/IMAGES/") // level 13
		{
			anisPath = PathManager::getAnimationSetPath(obj.imageSet);
		}
		else // level 7
		{
			anisPath = PathManager::getAnimationSetPath("LEVEL_ROCKSPRING");
		}
	}
	else if (obj.imageSet == "LEVEL_SPRINGBOX1") // level 9
	{
		anisPath = PathManager::getAnimationSetPath("LEVEL_SPRINGBOX");
	}
	else
	{
		anisPath = PathManager::getAnimationSetPath(obj.imageSet);
	}

	_idle = AssetsManager::loadAnimation(anisPath + "/IDLE.ANI", obj.imageSet);
	_spring = AssetsManager::loadCopyAnimation(anisPath + "/SPRING.ANI", obj.imageSet);

	_ani = _idle;
	_ani->position = position;
	_ani->updateImageData();
	Rectangle2D rc = _ani->GetRect();
	rc.left = position.x - 32;
	rc.right = position.x + 32;
	myMemCpy(_objRc, rc);
}

void SpringBoard::Logic(uint32_t elapsedTime)
{
	if (_ani != _spring && tryCatchPlayer())
	{
		_ani = _spring;
		_ani->reset();
		player->jump(_force);
	}

	if (_ani == _spring)
	{
		_ani->Logic(elapsedTime);
		if (_ani->isFinishAnimation())
			_ani = _idle;
	}
}
