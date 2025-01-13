#include "ConveyorBelt.h"
#include "../GlobalObjects.h"


#define ANIMATION_DURATION 100 // TODO: find perfect value (calc from speed)


ConveyorBelt::ConveyorBelt(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), speed(obj.speed / 1000.f),
	_canMoveCC(contains(obj.imageSet, "MIDDLE"))
{
	logicZ = DefaultZCoord::Characters + 1;
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), obj.speed > 0, ANIMATION_DURATION);
	if (_canMoveCC) logicZ -= 1; // handle the belts that can move CC first
	myMemCpy(_objRc, Rectangle2D((float)obj.moveRect.left, (float)obj.moveRect.top,
		(float)obj.moveRect.right, (float)obj.moveRect.bottom)); // I calc `moveRect` in `LevelPlane::updateObject`
}

void ConveyorBelt::Logic(uint32_t elapsedTime)
{
	if (tryCatchPlayer())
	{
		if (_canMoveCC)
			GO::getPlayerPosition().x += speed * elapsedTime;
	}

	_ani->Logic(elapsedTime);
}

// This function order the belts' animation frames (as sequence for long belts)
// The `belts` map contains the frame-number for each belt so we get sequence for long belts.
// (it comes as refrence from the ActionPlane)
void ConveyorBelt::orderAnimation(map<int, map<int, int>>& belts) {
	int framesCount = (int)_ani->getFramesCount();
	int x = (int)position.x / TILE_SIZE, y = (int)position.y / TILE_SIZE;

	// check if we have a belt on the left
	if (belts[y].count(x - 1) == 0)
		belts[y][x - 1] = 0; // dafault value

	int n = belts[y][x - 1] + 1; // get the value of left belt plus 1
	belts[y][x] = n;
	if (n > framesCount) n = n % framesCount; // loop animation
	if (speed > 0) n = framesCount - n; // reverse animation
	for (; n > 0; n--) _ani->advanceFrame(); // get to the right frame
}