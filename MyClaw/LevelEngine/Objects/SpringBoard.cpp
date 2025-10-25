#include "SpringBoard.h"
#include "../GlobalObjects.h"
#include "Player/Player.h"


SpringBoard::SpringBoard(const WwdObject& obj, int levelNumber)
	: BaseStaticPlaneObject(obj, false), _force(sqrt(2 * GRAVITY * (obj.maxY > 0 ? obj.maxY : 450)))
{
	string anisPath;
	if (obj.imageSet == "LEVEL_WATERROCK") // level 7,13
	{
		if (levelNumber == 13)
		{
			anisPath = PathManager::getAnimationSetPath(obj.imageSet);
		}
		else // if (levelNumber == 7)
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
		GO::player->jump(_force);
	}

	if (_ani == _spring)
	{
		_ani->Logic(elapsedTime);
		if (_ani->isFinishAnimation())
			_ani = _idle;
	}
}
