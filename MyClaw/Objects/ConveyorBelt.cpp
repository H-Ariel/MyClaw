#include "ConveyorBelt.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"


#define ANIMATION_DURATION 100 // TODO: find perfect value (calc from speed)

vector<ConveyorBelt*> ConveyorBelt::pConveyorBelts; // list of all conveyor belts in the current level


ConveyorBelt::ConveyorBelt(const WwdObject& obj, const WwdRect& rect)
	: BaseStaticPlaneObject(obj), speed(obj.speed / 1000.f),
	_canMoveCC(contains(obj.imageSet, "MIDDLE"))
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), ANIMATION_DURATION, obj.speed > 0);
	if (!_canMoveCC) (int32_t&)ZCoord += 1; // handle the belts that can move CC first
	float x = (float)obj.x - obj.x % TILE_SIZE, y = (float)obj.y - obj.y % TILE_SIZE;
	myMemCpy(_objRc, Rectangle2D(x + rect.left, y + rect.top, x + rect.right, y + rect.bottom));
	pConveyorBelts.push_back(this);
}

void ConveyorBelt::Logic(uint32_t elapsedTime)
{
	if (tryCatchPlayer())
	{
		if (_canMoveCC)
			player->position.x += speed * elapsedTime;
	}
		
	_ani->Logic(elapsedTime);
}

// This function order the belts' animation frames (as sequence for long belts)
void ConveyorBelt::GlobalInit()
{
	map<int, map<int, int>> belts; // [y][x] = ani frame number
	int x, y, i, n, fc;

	// sort belts by X position (left to right) beacuse we need to know if there is a belt on the left
	sort(pConveyorBelts.begin(), pConveyorBelts.end(), [](ConveyorBelt* a, ConveyorBelt* b) { return a->position.x < b->position.x; });

	// find all belts and their positions
	for (ConveyorBelt* b : pConveyorBelts)
	{
		x = (int)b->position.x / TILE_SIZE;
		y = (int)b->position.y / TILE_SIZE;
		belts[y][x] = 0;
	}

	// update belts' animation frames
	for (ConveyorBelt* b : pConveyorBelts)
	{
		fc = b->_ani->getImagesCount();

		x = (int)b->position.x / TILE_SIZE;
		y = (int)b->position.y / TILE_SIZE;

		// check if we have a belt on the left
		if (belts[y].count(x - 1) != 0)
		{
			n = belts[y][x - 1] + 1;
			belts[y][x] = n;

			if (n > fc) n = n % fc; // loop animation
			if (b->speed > 0) n = fc - n; // reverse animation
			for (i = 0; i < n; i++) b->_ani->Logic(ANIMATION_DURATION); // get to the right frame
		}
	}

	pConveyorBelts.clear(); // we don't need it anymore and we need it empty for next time we play this level
}
