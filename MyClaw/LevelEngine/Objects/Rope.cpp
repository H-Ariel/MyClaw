#include "Rope.h"
#include "Player/Player.h"
#include "../GlobalObjects.h"
#include "../CheatsManager.h"


constexpr float RECT_OFFSET = 5;


Rope::Rope(const WwdObject& obj)
	: BasePlaneObject(obj, false), _edgePos({})
{
	logicZ = DefaultZCoord::Characters + 1;
	_ani = AssetsManager::getAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), false, obj.speedX ? (obj.speedX / 60) : 25);
}

void Rope::Logic(uint32_t elapsedTime)
{
	// the edge position is where CC catch the rope (and where it ends in animation's frame)
	Rectangle2D aniRc = BasePlaneObject::GetRect();
	int n = (int)_ani->getFrameNumber();
	float progress = _ani->getFramesProgress();
	if (n == 29 || n == 30 || n == 88 || n == 89)
		_edgePos.x = (aniRc.left + aniRc.right) / 2; // for these frames rope ends in middle
	else if (progress <= 0.25f || 0.75 <= progress)
		_edgePos.x = aniRc.left;
	else
		_edgePos.x = aniRc.right;
	_edgePos.y = aniRc.bottom;

	if (!GO::player->isJumping() && GO::getPlayerRect().intersects(GetRect()) && !GO::cheats->isFlying())
	{
		GO::player->rope = this;
	}

	if (GO::player->rope == this)
	{
		// TODO maybe calculate according to speed and angle...

		GO::getPlayerPosition().x = _edgePos.x;
		GO::getPlayerPosition().y = _edgePos.y;

		if (GO::player->isMirrored())
		{
			GO::getPlayerPosition().x -= 2 * RECT_OFFSET;
		}
		else
		{
			GO::getPlayerPosition().x += 2 * RECT_OFFSET;
		}
	}
}
Rectangle2D Rope::GetRect()
{
	return Rectangle2D(
		_edgePos.x - RECT_OFFSET,
		_edgePos.y - RECT_OFFSET,
		_edgePos.x + RECT_OFFSET,
		_edgePos.y + RECT_OFFSET
	);
}
