#include "PhysicsManager.h"
#include "Player.h"
#include "GUI/WindowManager.h"
#include "LevelPlane.h"


#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x = collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define EMPTY_TILE -1


PhysicsManager::PhysicsManager(WapWorld* wwd, const LevelPlane* plane)
	: tiles(plane->tiles), tilesDescription(wwd->tilesDescription),
	tilesOnAxisX(plane->tilesOnAxisX), tilesOnAxisY(plane->tilesOnAxisY)
{
	/*
	TODO: the above code is only for `getEnemyRange` so we need 
	to delete this code and find new algorithm for `getEnemyRange`
	*/

	// map of all rectangles that BaseDynamicPlaneObjects can collide with 

	WwdTileDescription tileDesc;
	Rectangle2D tileRc;
	int i, j;
	float x1, x2, y1, y2;

	// add rectangle to list and merge it with previous rectangle if possible
	auto addRect = [&](const Rectangle2D& rc, uint32_t attr) {
		if (attr == WwdTileDescription::TileAttribute_Clear || rc.left == rc.right || rc.top == rc.bottom)
			return;

		pair<Rectangle2D, uint32_t> curr = { rc, attr };

		if (_rects.empty())
		{
			_rects.push_back(curr);
		}
		else
		{
			pair<Rectangle2D, uint32_t>& last = _rects.back();

			if (last.second == curr.second &&
				abs(last.first.top - curr.first.top) <= 2 &&
				abs(last.first.bottom - curr.first.bottom) <= 2 &&
				abs(last.first.right - curr.first.left) <= 2)
			{
				last.first = Rectangle2D(last.first.left, last.first.top, curr.first.right, last.first.bottom);
			}
			else
			{
				_rects.push_back(curr);
			}
		}
		};

	for (i = 0; i < tilesOnAxisY; i++)
	{
		for (j = 0; j < tilesOnAxisX; j++)
		{
			if (tiles[i][j] == EMPTY_TILE) tileDesc = {};
			else tileDesc = tilesDescription[tiles[i][j]];

			tileRc.left = (float)(j * TILE_SIZE);
			tileRc.top = (float)(i * TILE_SIZE);
			tileRc.right = tileRc.left + TILE_SIZE - 1;
			tileRc.bottom = tileRc.top + TILE_SIZE - 1;

			switch (tileDesc.type)
			{
			case WwdTileDescription::TileType_Single:
				addRect(tileRc, tileDesc.inAttr);
				break;

			case WwdTileDescription::TileType_Double:
				/*
				create 9 rectangles from `tileDesc.rect` and `tileRc`:
				all rectangles create `tileRc` except the middle one which creates `tileDesc.rect`
				o | o | o
				--+---+--
				o | i | o
				--+---+--
				o | o | o
				(o - outside, i - inside)
				*/
				x1 = tileRc.left + tileDesc.rect.left;
				x2 = tileRc.left + tileDesc.rect.right;
				y1 = tileRc.top + tileDesc.rect.top;
				y2 = tileRc.top + tileDesc.rect.bottom;
				addRect({ tileRc.left, tileRc.top, x1, y1 }, tileDesc.outAttr);
				addRect({ x1, tileRc.top, x2, y1 }, tileDesc.outAttr);
				addRect({ x2, tileRc.top, tileRc.right, y1 }, tileDesc.outAttr);
				addRect({ tileRc.left, y1, x1, y2 }, tileDesc.outAttr);
				addRect({ x1, y1, x2, y2 }, tileDesc.inAttr);
				addRect({ x2, y1, tileRc.right, y2 }, tileDesc.outAttr);
				addRect({ tileRc.left, y2, x1, tileRc.bottom }, tileDesc.outAttr);
				addRect({ x1, y2, x2, tileRc.bottom }, tileDesc.outAttr);
				addRect({ x2, y2, tileRc.right, tileRc.bottom }, tileDesc.outAttr);
				break;
			}
		}
	}

	// this loop combines rectangles that are next to each other (according to x axis)
	for (i = 0; i < _rects.size(); i++)
	{
		for (j = i + 1; j < _rects.size(); j++)
		{
			if (_rects[i].second == _rects[j].second &&
				abs(_rects[i].first.top - _rects[j].first.top) <= 2 &&
				abs(_rects[i].first.bottom - _rects[j].first.bottom) <= 2 &&
				abs(_rects[i].first.right - _rects[j].first.left) <= 2)
			{
				_rects[i].first = Rectangle2D(_rects[i].first.left, _rects[i].first.top, _rects[j].first.right, _rects[i].first.bottom);
				_rects.erase(_rects.begin() + j);
				j--;
			}
		}
	}

	// this loop combines rectangles that are next to each other (according to y axis)
	for (i = 0; i < _rects.size(); i++)
	{
		for (j = i + 1; j < _rects.size(); j++)
		{
			if (_rects[i].second == _rects[j].second &&
				abs(_rects[i].first.left - _rects[j].first.left) <= 1 &&
				abs(_rects[i].first.right - _rects[j].first.right) <= 1 &&
				abs(_rects[i].first.bottom - _rects[j].first.top) <= 1)
			{
				_rects[i].first = Rectangle2D(_rects[i].first.left, _rects[i].first.top, _rects[i].first.right, _rects[j].first.bottom);
				_rects.erase(_rects.begin() + j);
				j--;
			}
		}
	}
}

// TODO: use these functions. the problem: CC can't jump. need check for all objects
void PhysicsManager::moveX(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const
{
	obj->position.x += obj->speed.x * elapsedTime;
	checkCollides(obj);
}
void PhysicsManager::moveY(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const
{
	obj->position.y += obj->speed.y * elapsedTime;
	checkCollides(obj);
}
void PhysicsManager::move(BaseDynamicPlaneObject* obj, uint32_t elapsedTime) const
{
	obj->position.x += obj->speed.x * elapsedTime;
	obj->position.y += obj->speed.y * elapsedTime;
	checkCollides(obj);
}

void PhysicsManager::checkCollides(BaseDynamicPlaneObject* obj) const
{
	// TODO: better code here (the logic is good, but the code is not)


	WwdTileDescription tileDesc;
	const Rectangle2D objRc = obj->GetRect();
	Rectangle2D collisions[9];
	Rectangle2D cumulatedCollision, tileRc, collisionRc;
	int collisionsNumber = 0, i, j;
	const int minX = max((int)objRc.left / TILE_SIZE - 1, 0);
	const int maxX = min((int)objRc.right / TILE_SIZE + 1, tilesOnAxisX - 1);
	const int minY = max((int)objRc.top / TILE_SIZE - 1, 0);
	const int maxY = min((int)objRc.bottom / TILE_SIZE + 1, tilesOnAxisY - 1);
	float x0, x1, x2, x3;
	float y0, y1, y2, y3;
	const bool isPlayer = isinstance<Player>(obj);

	auto _addCollision = [&]() { // add `collisionRc` to the `cumulatedCollision`
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
	auto _onGround = [&]() {
		float smallestBottom = collisionRc.getSmallest().bottom;
		// when object is falling or when CC climb down and arrive to the ground
		if ((obj->isFalling() || (isPlayer && BasePlaneObject::player->isClimbing())) &&
			(collisionRc.right > 0 || collisionRc.left > 0) && (0 < smallestBottom && smallestBottom < 16))
		{
			_addCollision();
		}
	};
	auto _onLadder = [&]() {
		// check if object is at the top of the ladder, so it should stay here (and not fall)
		bool isOnLadderTop = false;

		if (i > 1 && tilesDescription[tiles[i][j]].inAttr == WwdTileDescription::TileAttribute_Climb)
		{
			int id = tiles[i - 1][j]; // tile above
			if (id == EMPTY_TILE)
			{
				isOnLadderTop = true;
			}
			else if (tilesDescription[id].inAttr != WwdTileDescription::TileAttribute_Climb)
			{
				isOnLadderTop = true;
			}
		}

		if (isPlayer)
			isOnLadderTop = !BasePlaneObject::player->isClimbing() && isOnLadderTop;

		if (isOnLadderTop)
			_onGround(); // ladder top behaves like ground

		if (isPlayer) // let Captain Claw climb
		{
			if (!collisionRc.getSmallest().isEmpty())
			{
				BasePlaneObject::player->setLadderFlags(isOnLadderTop);
				if (BasePlaneObject::player->isClimbing())
				{
					// set the player position on the ladder easily for the user
					BasePlaneObject::player->position.x = (tileRc.left + tileRc.right) / 2;
				}
			}
		}
	};

	auto addRect = [&](uint32_t attr) {
		if (attr == WwdTileDescription::TileAttribute_Clear ||
			tileRc.left == tileRc.right || tileRc.top == tileRc.bottom)
			return;

		collisionRc = objRc.getCollision(tileRc);
		switch (attr)
		{
		case WwdTileDescription::TileAttribute_Clear: break;
		case WwdTileDescription::TileAttribute_Solid: _addCollision(); break;
		case WwdTileDescription::TileAttribute_Ground: _onGround(); break;
		case WwdTileDescription::TileAttribute_Climb: _onLadder(); break;
		case WwdTileDescription::TileAttribute_Death: obj->whenTouchDeath(); break;
		}
	};

	for (i = minY; i < maxY; i++)
	{
		for (j = minX; j < maxX; j++)
		{
			if (tiles[i][j] == EMPTY_TILE)
				continue;
			
			tileDesc = tilesDescription[tiles[i][j]];

			tileRc.left = (float)(j * TILE_SIZE);
			tileRc.top = (float)(i * TILE_SIZE);
			tileRc.right = tileRc.left + TILE_SIZE - 1;
			tileRc.bottom = tileRc.top + TILE_SIZE - 1;

			switch (tileDesc.type)
			{
			case WwdTileDescription::TileType_Single:
				addRect(tileDesc.inAttr);
				break;

			case WwdTileDescription::TileType_Double:
				/*
				create 9 rectangles from `tileDesc.rect` and `tileRc`:
				all rectangles create `tileRc` except the middle one which creates `tileDesc.rect`
				o | o | o
				--+---+--
				o | i | o
				--+---+--
				o | o | o
				(o - outside, i - inside)
				*/

				x0 = tileRc.left;
				x1 = tileRc.left + tileDesc.rect.left;
				x2 = tileRc.left + tileDesc.rect.right;
				x3 = tileRc.right;
				y0 = tileRc.top;
				y1 = tileRc.top + tileDesc.rect.top;
				y2 = tileRc.top + tileDesc.rect.bottom;
				y3 = tileRc.bottom;

				tileRc = { x0, y0, x1, y1 }; addRect(tileDesc.outAttr);
				tileRc = { x1, y0, x2, y1 }; addRect(tileDesc.outAttr);
				tileRc = { x2, y0, x3, y1 }; addRect(tileDesc.outAttr);
				tileRc = { x0, y1, x1, y2 }; addRect(tileDesc.outAttr);
				tileRc = { x1, y1, x2, y2 }; addRect(tileDesc.inAttr);
				tileRc = { x2, y1, x3, y2 }; addRect(tileDesc.outAttr);
				tileRc = { x0, y2, x1, y3 }; addRect(tileDesc.outAttr);
				tileRc = { x1, y2, x2, y3 }; addRect(tileDesc.outAttr);
				tileRc = { x2, y2, x3, y3 }; addRect(tileDesc.outAttr);

				break;
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

pair<float, float> PhysicsManager::getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const
{
	// TODO: update this function

	Rectangle2D enemyRect(enemyPos.x, enemyPos.y, enemyPos.x, enemyPos.y + 80);

	for (auto& p : _rects)
	{
		if ((p.second == WwdTileDescription::TileAttribute_Solid || p.second == WwdTileDescription::TileAttribute_Ground)
			&& p.first.intersects(enemyRect))
		{
			float left = p.first.left + 32, right = p.first.right - 32;

			if (minX != 0 && maxX != 0)
			{
				left = max(left, minX);
				right = min(right, maxX);
			}

			return { left, right };
		}
	}

	return { enemyPos.x - 32, enemyPos.x + 32 };
}
