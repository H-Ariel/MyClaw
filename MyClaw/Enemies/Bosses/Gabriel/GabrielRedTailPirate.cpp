#include "GabrielRedTailPirate.h"
#include "../../RedTailPirate.h"
#include "../../../GlobalObjects.h"


GabrielRedTailPirate::GabrielRedTailPirate()
	: BaseDynamicPlaneObject({}), _isJumping(false)
{
	position = { 43479, 5020 };
	logicZ = DefaultZCoord::Characters;
	drawZ = DefaultZCoord::Characters;
	speed.x = -0.3f; // run to left t CC's dock
	_ani = AssetsManager::loadCopyAnimation("LEVEL8/ANIS/REDTAILPIRATE/ADVANCE.ANI", "LEVEL8/IMAGES/REDTAILPIRATE");
}
void GabrielRedTailPirate::Logic(uint32_t elapsedTime)
{
	speed.y += GRAVITY * elapsedTime;
	position.x += speed.x * elapsedTime;
	position.y += speed.y * elapsedTime;
	if (position.x < 43200 && !_isJumping)
	{
		speed.y = -0.7f;
		_isJumping = true;
		position.y -= 1;
		_ani->updateFrames = false;
	}
	_ani->position = position;
	_ani->Logic(elapsedTime);
}
void GabrielRedTailPirate::stopFalling(float collisionSize)
{
	// if he stop falls after jumping it means he arrived to
	// CC's dock, so we swap this object with regular enemy
	if (_isJumping)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.minX = 42560;
		obj.maxX = 43200;
		obj.imageSet = "LEVEL_REDTAILPIRATE";
		GO::addObjectToActionPlane(DBG_NEW RedTailPirate(obj, true));

		removeObject = true;
	}
	else
	{
		BaseDynamicPlaneObject::stopFalling(collisionSize);
	}
}
// they always run (until they jump at end of dock)
void GabrielRedTailPirate::stopMovingLeft(float collisionSize) { }
void GabrielRedTailPirate::stopMovingRight(float collisionSize) { }
