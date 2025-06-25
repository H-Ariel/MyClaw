#include "AquatisStalactite.h"


// this object same to regular stalactite but with different logic
AquatisStalactite::AquatisStalactite(const WwdObject& obj)
	: Projectile(obj, "")
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), false, 50);
	_ani->updateFrames = false;
	speed = {};
}
void AquatisStalactite::Logic(uint32_t elapsedTime)
{
	if (speed.y != 0)
	{
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
	}

	_ani->Logic(elapsedTime);
	removeObject = _ani->isFinishAnimation();
}
void AquatisStalactite::stopFalling(float collisionSize) { _ani->updateFrames = true; }
int AquatisStalactite::getDamage() const { return 10; }
void AquatisStalactite::activate() { speed.y = 0.25f; } // drops the stalactite towards Aquatis
