#include "ConveyorBelt.h"
#include "../GlobalObjects.h"
#include "Player.h"


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

// TODO: maybe combine each row of belts to one ln belt... this will save lot of troubles

void ConveyorBelt::Logic(uint32_t elapsedTime)
{
	auto b = [&]() { // changes for `tryCatchPlayer` to found perfect logic
		//if (GO::player->isFalling())
		{
			Rectangle2D colRc = GO::GO::getPlayerRect().getCollision(GetRect());
			float smallestBottom = colRc.getSmallest().bottom;
			if ((colRc.right > 0 || colRc.left > 0) && (0 < smallestBottom && smallestBottom < 16))
			{
				// if player is falling/going to this object - catch him
				//GO::player->stopFalling(colRc.bottom);
				return true;
			}
		}
		return false;
	};

	//if (tryCatchPlayer())
	if (b())
	{
		if (_canMoveCC)
			GO::getPlayerPosition().x += speed * elapsedTime;
	}

	_ani->Logic(elapsedTime);
}

// This function order the belts' animation frames (as sequence for long
// belts). `n` indicate the frame-number for each belt so we get sequence
// for long belts (it comes as refrence from the ActionPlane)
void ConveyorBelt::orderAnimation(int n) {
	int framesCount = (int)_ani->getFramesCount();
	n = n % framesCount; // keep modulo to repeat animation for sequence
	if (speed > 0) n = framesCount - n; // reverse animation if needed
	for (; n > 0; n--) _ani->advanceFrame(); // get to the right frame
}