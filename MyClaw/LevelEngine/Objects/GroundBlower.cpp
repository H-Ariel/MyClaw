#include "GroundBlower.h"
#include "../GlobalObjects.h"
#include "Player/Player.h"


constexpr float BLOWHOLE_OFFSET_Y = 120;// offset for the blowhole


GroundBlower::GroundBlower(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _force(sqrt(2 * GRAVITY * (obj.maxY > 0 ? obj.maxY : 450)))
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_GROUNDBLOWER"));
	_blowhole = AssetsManager::getAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_BLOW1"));

	Rectangle2D rc;
	rc.left = position.x - 24;
	rc.right = position.x + 24;
	rc.bottom = position.y + BLOWHOLE_OFFSET_Y - 40;
	rc.top = rc.bottom - 1;
	myMemCpy(_objRc, rc);
	logicZ = DefaultZCoord::Characters - 1; // avoid the affect of PhysicsManager on the player
}

void GroundBlower::Logic(uint32_t elapsedTime)
{
	if (_ani->getFrameNumber() == 0)
	{
		Rectangle2D colRc = GO::getPlayerRect().getCollision(GetRect());
		if (!colRc.isEmpty())
		{
			// if player is falling/going to this object - catch him and blow him up
			GO::player->stopFalling(colRc.bottom);
			GO::player->jump(_force);
		}
	}
}

void GroundBlower::Draw()
{
	_ani->position = position;
	_ani->mirrored = _isMirrored;
	_ani->Draw();

	_blowhole->position = position;
	_blowhole->position.y += BLOWHOLE_OFFSET_Y;
	_blowhole->mirrored = _isMirrored;
	_blowhole->Draw();
}
