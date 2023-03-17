#include "SpringBoard.h"
#include "../Player.h"
#include "../AssetsManager.h"


SpringBoard::SpringBoard(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _force(sqrt(2 * GRAVITY * (obj.maxY > 0 ? obj.maxY : 450)))
{
	string anisPath;
	if (obj.imageSet == "LEVEL_WATERROCK") // level 7. TODO: something else
		anisPath = PathManager::getAnimationSetPath("LEVEL_ROCKSPRING");
	else if (obj.imageSet == "LEVEL_SPRINGBOX1") // level 9. TODO: something else
		anisPath = PathManager::getAnimationSetPath("LEVEL_SPRINGBOX");
	else
		anisPath = PathManager::getAnimationSetPath(obj.imageSet);

	_idle = AssetsManager::loadAnimation(anisPath + "/IDLE.ANI", obj.imageSet);
	_idle->updateFrames = false;
	_idle->position = position;
	_spring = AssetsManager::loadCopyAnimation(anisPath + "/SPRING.ANI", obj.imageSet);
	_spring->position = position;

	_ani = _idle;

	_ani->updateImageData();
	D2D1_RECT_F rc = _ani->GetRect();

	if (obj.imageSet == "SPRINGYFERN") // level 4. TODO: something else
	{
		rc.left += 40;
		rc.right -= 40;
	}

	myMemCpy(_objRc, rc);
}

void SpringBoard::Logic(uint32_t elapsedTime)
{
	if (_ani != _spring && tryCatchPlayer())
	{
		_ani = _spring;
		_ani->reset();
		_player->jump(_force);
	}

	if (_ani == _spring)
	{
		_ani->Logic(elapsedTime);
		if (_ani->isFinishAnimation())
			_ani = _idle;
	}
}
