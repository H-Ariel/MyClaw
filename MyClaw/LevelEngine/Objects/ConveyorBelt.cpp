#include "ConveyorBelt.h"
#include "../GlobalObjects.h"
#include "Player.h"


constexpr int ANIMATION_DURATION = 100; // TODO: find perfect value (calc from speed)


ConveyorBelt::ConveyorBelt(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _speed(obj.speed / 1000.f),
	_canMoveCC(contains(obj.imageSet, "MIDDLE"))
{
	logicZ = DefaultZCoord::Characters + 1;
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), obj.speed > 0, ANIMATION_DURATION);
	if (_canMoveCC) logicZ -= 1; // handle the belts that can move CC first
	myMemCpy(_objRc, Rectangle2D((float)obj.moveRect.left, (float)obj.moveRect.top,
		(float)obj.moveRect.right, (float)obj.moveRect.bottom)); // I calc `moveRect` in `LevelPlane::updateObject`

	_animations.push_back(_ani);
}

void ConveyorBelt::Logic(uint32_t elapsedTime)
{
	Rectangle2D colRc = GO::getPlayerRect().getCollision(GetRect());
	float smallestBottom = colRc.getSmallest().bottom;
	if ((colRc.right > 0 || colRc.left > 0) && (0 < smallestBottom && smallestBottom < 16))
	{
		// if player is falling/going to this object - catch him
		GO::player->conveyorBelt = this;
	}

	for (auto& a : _animations)
		a->Logic(elapsedTime);
}

void ConveyorBelt::Draw()
{
	D2D1_POINT_2F pos = position;
	for (auto& a : _animations)
	{
		a->position = pos;
		a->Draw();
		pos.x += TILE_SIZE;
	}
}

bool ConveyorBelt::hasSameMovement(ConveyorBelt* belt) const
{
	return this->_canMoveCC == belt->_canMoveCC && this->_speed == belt->_speed;
}

// This function connects two belts into one long belt (used in `ActionPlane::joinConveyorBelts`)
void ConveyorBelt::extend(ConveyorBelt* belt)
{
	myMemCpy(_objRc, Rectangle2D(
		min(_objRc.left, belt->_objRc.left),
		min(_objRc.top, belt->_objRc.top),
		max(_objRc.right, belt->_objRc.right),
		max(_objRc.bottom, belt->_objRc.bottom)
	));

	_animations.push_back(belt->_ani);

	int framesCount = (int)_ani->getFramesCount();
	int n = _animations.size() % framesCount; // keep modulo to repeat animation for sequence

	if (_speed > 0)
		n = framesCount - n; // reverse animation if needed

	for (; n > 0; n--) // get to the right frame
		_animations.back()->advanceFrame();
}
