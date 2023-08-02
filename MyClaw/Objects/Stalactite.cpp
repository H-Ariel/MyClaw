#include "Stalactite.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Assets-Managers/PathManager.h"
#include "../Player.h"


Stalactite::Stalactite(const WwdObject& obj)
	: Projectile(obj, "", "") // the stalactite behave like a projectile
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 50, false);
	_ani->updateFrames = false;

	_activityArea.left = (float)obj.userRect1.left;
	_activityArea.top = (float)obj.userRect1.top;
	_activityArea.right = (float)obj.userRect1.right;
	_activityArea.bottom = (float)obj.userRect1.bottom;

	speed = {};
}

void Stalactite::Logic(uint32_t elapsedTime)
{
	if (speed.y == 0)
	{
		if (_activityArea.intersects(player->GetRect()))
			speed.y = 0.25f;
	}
	else
	{
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
	}

	_ani->position = position;
	_ani->Logic(elapsedTime);
	removeObject = _ani->isFinishAnimation();
}

void Stalactite::stopFalling(float collisionSize)
{
	_ani->updateFrames = true;
}

int Stalactite::getDamage() const
{
	return 10; // TODO: find the real damage
}
