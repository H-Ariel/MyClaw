#include "GroundBlower.h"
#include "../Player.h"
#include "../AssetsManager.h"


#define BLOWHOLE_OFFSET_Y 120 // offset for the blowhole


GroundBlower::GroundBlower(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _force(sqrt(2 * GRAVITY * (obj.maxY > 0 ? obj.maxY : 450)))
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_GROUNDBLOWER"));
	_blowhole = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_BLOW1"), 125, false);

	D2D1_RECT_F rc = {};
	rc.left = position.x - 24;
	rc.right = position.x + 24;
	rc.bottom = rc.top = position.y + BLOWHOLE_OFFSET_Y - 40;
	myMemCpy(_objRc, rc);
}

void GroundBlower::Logic(uint32_t elapsedTime)
{
	if (_ani->getFrameNumber() == 0 && tryCatchPlayer())
	{
		_player->jump(_force);
	}
}

void GroundBlower::Draw()
{
	_ani->position = position;
	_ani->mirrored = _isMirrored;
	_ani->updateImageData();
	_ani->Draw();

	_blowhole->position = position;
	_blowhole->position.y += BLOWHOLE_OFFSET_Y;
	_blowhole->mirrored = _isMirrored;
	_blowhole->updateImageData();
	_blowhole->Draw();
}
