#include "PhysicsManager.h"
#include "Player.h"
#include "WindowManager.h"


#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x += collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define EMPTY_TILE -1


// TODO: add function to update object position


const float PhysicsManager::myGRAVITY = GRAVITY;

PhysicsManager::PhysicsManager(const WwdPlane& plane, const shared_ptr<WapWorld>& wwd, Player*& player)
	: _plane(plane), _wwd(wwd), _player(player)
{
}

void reduceRectangles(vector<Rectangle2D>& rects)
{
	size_t i, j;

	// this loop round all rectangles edges
	for (Rectangle2D& r : rects)
	{
		if ((int32_t)r.left % 64 == 63) r.left += 1;
		if ((int32_t)r.top % 64 == 63) r.top += 1;
		if ((int32_t)r.right % 64 == 63) r.right += 1;
		if ((int32_t)r.bottom % 64 == 63) r.bottom += 1;
	}

	// this loop combines rectangles that are next to each other (according to x axis)
	for (i = 0; i < rects.size(); i++)
	{
		for (j = i + 1; j < rects.size(); j++)
		{
			if (rects[i].top == rects[j].top && rects[i].bottom == rects[j].bottom &&
				rects[i].right == rects[j].left || rects[i].right + 1 == rects[j].left)
			{
				Rectangle2D newRc(rects[i].left, rects[i].top, rects[j].right, rects[i].bottom);
				rects.erase(rects.begin() + j);
				rects[i] = newRc;
				j--;
			}
		}
	}

	// this loop combines rectangles that are next to each other (according to y axis)
	for (i = 0; i < rects.size(); i++)
	{
		for (j = i + 1; j < rects.size(); j++)
		{
			if (rects[i].left == rects[j].left && rects[i].right == rects[j].right &&
				rects[i].bottom == rects[j].top || rects[i].bottom + 1 == rects[j].top)
			{
				Rectangle2D newRc(rects[i].left, rects[i].top, rects[i].right, rects[j].bottom);
				rects.erase(rects.begin() + j);
				rects[i] = newRc;
				j--;
			}
		}
	}
}

void PhysicsManager::init()
{
	// map of all rectangles that player can collide with 

	Rectangle2D tileRc, originalTileRc, rc1, rc2;
	uint32_t i, j;

	for (i = 0; i < _plane.tilesOnAxisY; i++)
	{
		for (j = 0; j < _plane.tilesOnAxisX; j++)
		{
			const WwdTileDescription& tileDesc = _wwd->tilesDescription[_plane.tiles[i][j]];

			tileRc.left = (float)(j * _plane.tilePixelWidth);
			tileRc.top = (float)(i * _plane.tilePixelHeight);
			tileRc.right = tileRc.left + _plane.tilePixelWidth;
			tileRc.bottom = tileRc.top + _plane.tilePixelHeight;

			switch (tileDesc.type)
			{
			case WwdTileDescription::TileType_Single: {
				switch (tileDesc.insideAttrib)
				{
				case WwdTileDescription::TileAttribute_Solid:
					_solidRects.push_back(tileRc);
					break;

				case WwdTileDescription::TileAttribute_Ground:
					_groundRects.push_back(tileRc);
					break;

				case WwdTileDescription::TileAttribute_Climb:
					_climbRects.push_back(tileRc);
					break;

				case WwdTileDescription::TileAttribute_Death:
					_deathRects.push_back(tileRc);
					break;
				}
			}	break;

			case WwdTileDescription::TileType_Double: { // TODO: improve this part
				originalTileRc = tileRc;

				tileRc.left += tileDesc.rect.left;
				tileRc.top += tileDesc.rect.top;
				tileRc.right = tileRc.right + tileDesc.rect.right - tileDesc.width;
				tileRc.bottom = tileRc.bottom + tileDesc.rect.bottom - tileDesc.height;

				switch (tileDesc.insideAttrib)
				{
				case WwdTileDescription::TileAttribute_Solid:
					_solidRects.push_back(tileRc);
					break;
				case WwdTileDescription::TileAttribute_Ground:
					_groundRects.push_back(tileRc);
					break;
				case WwdTileDescription::TileAttribute_Climb:
					_climbRects.push_back(tileRc);
					break;
				case WwdTileDescription::TileAttribute_Death:
					_deathRects.push_back(tileRc);
					break;
				}

				if (tileDesc.outsideAttrib == WwdTileDescription::TileAttribute_Solid)
				{
					if (tileDesc.rect.right == tileDesc.width - 1 && tileDesc.rect.top == 0)
					{
						rc1 = { originalTileRc.left, originalTileRc.top, tileRc.left, originalTileRc.bottom };
						rc2 = { tileRc.left, tileRc.bottom, tileRc.right, originalTileRc.bottom };
					}
					else if (tileDesc.rect.left == 0 && tileDesc.rect.top == 0)
					{
						rc1 = { tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom };
						rc2 = { originalTileRc.left, tileRc.bottom, originalTileRc.right, originalTileRc.bottom };
					}
					else
					{
						continue;
						// TODO: handle the other case
					}

					_solidRects.push_back(rc1);
					_solidRects.push_back(rc2);
				}
			}	break;

			default: break;
			}
		}
	}


	cout << "Total amount of rectangles: " << _solidRects.size() + _groundRects.size() + _climbRects.size() + _deathRects.size() << endl;

	reduceRectangles(_solidRects);
	reduceRectangles(_groundRects);
	reduceRectangles(_climbRects);
	reduceRectangles(_deathRects);

	cout << "Total amount of rectangles after reduce: " << _solidRects.size() + _groundRects.size() + _climbRects.size() + _deathRects.size() << endl;
}

void PhysicsManager::Draw()
{
	for (Rectangle2D& rc : _solidRects)
		WindowManager::drawRect(rc, ColorF::Red);
	for (Rectangle2D& rc : _groundRects)
		WindowManager::drawRect(rc, ColorF::Magenta);
	for (Rectangle2D& rc : _climbRects)
		WindowManager::drawRect(rc, ColorF::Green);
	for (Rectangle2D& rc : _deathRects)
		WindowManager::drawRect(rc, ColorF::Blue);
}

void PhysicsManager::checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath)
{
	// TODO: _solidRects, _groundRects, _deathRects, _climbRects

	static const float N = 2.5f; // this number indicate how many tile we will check from every side
	const Rectangle2D objRc = obj->GetRect();
	const uint32_t MinXPos = (uint32_t)(obj->position.x / _plane.tilePixelWidth - N);
	const uint32_t MaxXPos = (uint32_t)(obj->position.x / _plane.tilePixelWidth + N);
	const uint32_t MinYPos = (uint32_t)(obj->position.y / _plane.tilePixelHeight - N);
	const uint32_t MaxYPos = (uint32_t)(obj->position.y / _plane.tilePixelHeight + N);

	Rectangle2D collisions[9];
	Rectangle2D cumulatedCollision, tileRc;
	Rectangle2D collisionRc;
	Rectangle2D originalTileRc, rc1, rc2;
	uint32_t i, j, collisionsNumber = 0;

	const bool isPlayer = isinstance<Player>(obj);


	auto _addCollision = [&]() { // add `collisionRect` to the `cumulatedCollision`
		if (!collisionRc.isEmpty())
		{
			// add this collision to the list
			collisions[collisionsNumber] = collisionRc;
			collisions[collisionsNumber].keepSmallest();
			// add the collision details to the cummulated collision
			_checkCollides_define1(top);
			_checkCollides_define1(bottom);
			_checkCollides_define1(left);
			_checkCollides_define1(right);
			collisionsNumber++;
		}
	};
	auto _onSolid = [&](Rectangle2D tileRect) {
		collisionRc = objRc.getCollision(tileRect);
		_addCollision();
	};
	auto _onGround = [&]() { // same to `BasePlaneObject::tryCatchPlayer`
		collisionRc = objRc.getCollision(tileRc);
		if (collisionRc.getSmallest().bottom > 0 && (collisionRc.right > 0 || collisionRc.left > 0) && obj->isFalling())
		{
			_addCollision();
		}
	};
	auto _onLadder = [&]() {
		if (objRc.intersects(tileRc))
		{
			bool isOnLadderTop = false;

			// check if object is at the top of the ladder, so it should stay here
			if (i > 0)
			{
				int32_t aboveTileId = _plane.tiles[size_t(i - 1)][j];
				isOnLadderTop = (aboveTileId == EMPTY_TILE || (
					_wwd->tilesDescription[aboveTileId].insideAttrib != WwdTileDescription::TileAttribute_Climb &&
					_wwd->tilesDescription[aboveTileId].outsideAttrib != WwdTileDescription::TileAttribute_Climb));

				if (isPlayer)
				{
					isOnLadderTop = !_player->isClimbing() && isOnLadderTop;
				}
			}

			if (isOnLadderTop)
			{
				_onGround(); // ladder top behaves like ground
			}

			if (isPlayer) // let Captain Claw climb
			{
				_player->setLadderFlags(isOnLadderTop);
				if (_player->isClimbing())
				{
					// set the player position on the ladder easily for the user
					_player->position.x = (tileRc.left + tileRc.right) / 2;
				}
			}
		}
	};
	auto _onDeath = [&]() {
		if (objRc.intersects(tileRc))
		{
			whenTouchDeath();
		}
	};

	for (i = MinYPos; i < MaxYPos && i < _plane.tilesOnAxisY; i++)
	{
		for (j = MinXPos; j < MaxXPos && j < _plane.tilesOnAxisX; j++)
		{
			const WwdTileDescription& tileDesc = _wwd->tilesDescription[_plane.tiles[i][j]];

			tileRc.left = (float)(j * _plane.tilePixelWidth);
			tileRc.top = (float)(i * _plane.tilePixelHeight);
			tileRc.right = tileRc.left + _plane.tilePixelWidth;
			tileRc.bottom = tileRc.top + _plane.tilePixelHeight;

			switch (tileDesc.type)
			{
			case WwdTileDescription::TileType_Single: {
				switch (tileDesc.insideAttrib)
				{
				case WwdTileDescription::TileAttribute_Solid:
					_onSolid(tileRc);
					break;

				case WwdTileDescription::TileAttribute_Ground:
					_onGround();
					break;

				case WwdTileDescription::TileAttribute_Climb:
					_onLadder();
					break;

				case WwdTileDescription::TileAttribute_Death: // this case is the purple rectangles
					_onDeath();
					break;
				}
			}	break;

			case WwdTileDescription::TileType_Double: { // TODO: improve this part
				originalTileRc = tileRc;

				tileRc.left += tileDesc.rect.left;
				tileRc.top += tileDesc.rect.top;
				tileRc.right = tileRc.right + tileDesc.rect.right - tileDesc.width;
				tileRc.bottom = tileRc.bottom + tileDesc.rect.bottom - tileDesc.height;

				switch (tileDesc.insideAttrib)
				{
				case WwdTileDescription::TileAttribute_Solid: // this case is the green rectangles
					_onSolid(tileRc);
					break;
				case WwdTileDescription::TileAttribute_Ground: // this case is the magenta rectangles
					_onGround();
					break;
				case WwdTileDescription::TileAttribute_Climb:
					_onLadder();
					break;
				case WwdTileDescription::TileAttribute_Death: // also this case is the purple rectangles
					_onDeath();
					break;
				}

				if (tileDesc.outsideAttrib == WwdTileDescription::TileAttribute_Solid) // this case is the yellow rectangles
				{
					if (tileDesc.rect.right == tileDesc.width - 1 && tileDesc.rect.top == 0)
					{
						rc1 = { originalTileRc.left, originalTileRc.top, tileRc.left, originalTileRc.bottom };
						rc2 = { tileRc.left, tileRc.bottom, tileRc.right, originalTileRc.bottom };
					}
					else if (tileDesc.rect.left == 0 && tileDesc.rect.top == 0)
					{
						rc1 = { tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom };
						rc2 = { originalTileRc.left, tileRc.bottom, originalTileRc.right, originalTileRc.bottom };
					}
					else
					{
						continue;
						// TODO: handle the other case
					}

					_onSolid(rc1);
					_onSolid(rc2);
				}
			}	break;

			default: break;
			}
		}
	}

	// whichever side collides the most, that side is taken into consideration
	cumulatedCollision.keepLargest();
	_checkCollides_define2(top);
	_checkCollides_define2(bottom);
	_checkCollides_define2(left);
	_checkCollides_define2(right);

	if (cumulatedCollision.top > 0) obj->bounceTop();
	else if (cumulatedCollision.bottom > 0) obj->stopFalling(cumulatedCollision.bottom);
	else if (cumulatedCollision.left > 0) obj->stopMovingLeft(cumulatedCollision.left);
	else if (cumulatedCollision.right > 0) obj->stopMovingRight(cumulatedCollision.right);
}
