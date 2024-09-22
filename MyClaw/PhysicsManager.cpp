#include "PhysicsManager.h"
#include "Player.h"
#include "GameEngine/WindowManager.h"
#include "LevelPlane.h"


#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x = collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));


PhysicsManager::PhysicsManager(WapWwd* wwd, const LevelPlane* plane)
{
	// map of all rectangles that BaseDynamicPlaneObjects can collide with 

	WwdTileDescription tileDesc;
	Rectangle2D tileRc;
	uint32_t i, j;
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

	for (i = 0; i < plane->_wwdPlane->tilesOnAxisY; i++)
	{
		for (j = 0; j < plane->_wwdPlane->tilesOnAxisX; j++)
		{
			if (plane->_wwdPlane->tiles[i][j] == EMPTY_TILE) tileDesc = {};
			else tileDesc = wwd->tileDescriptions[plane->_wwdPlane->tiles[i][j]];

			tileRc.left = (float)(j * TILE_SIZE);
			tileRc.top = (float)(i * TILE_SIZE);
			tileRc.right = tileRc.left + TILE_SIZE - 1;
			tileRc.bottom = tileRc.top + TILE_SIZE - 1;

			switch (tileDesc.type)
			{
			case WwdTileDescription::TileType_Single:
				addRect(tileRc, tileDesc.insideAttrib);
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
				addRect({ tileRc.left, tileRc.top, x1, y1 }, tileDesc.outsideAttrib);
				addRect({ x1, tileRc.top, x2, y1 }, tileDesc.outsideAttrib);
				addRect({ x2, tileRc.top, tileRc.right, y1 }, tileDesc.outsideAttrib);
				addRect({ tileRc.left, y1, x1, y2 }, tileDesc.outsideAttrib);
				addRect({ x1, y1, x2, y2 }, tileDesc.insideAttrib);
				addRect({ x2, y1, tileRc.right, y2 }, tileDesc.outsideAttrib);
				addRect({ tileRc.left, y2, x1, tileRc.bottom }, tileDesc.outsideAttrib);
				addRect({ x1, y2, x2, tileRc.bottom }, tileDesc.outsideAttrib);
				addRect({ x2, y2, tileRc.right, tileRc.bottom }, tileDesc.outsideAttrib);
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
	if (obj->speed.y > 1.5f) obj->speed.y = 1.5f;
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
	const Rectangle2D objRc = obj->GetRect();
	Rectangle2D collisions[9];
	Rectangle2D cumulatedCollision, tileRc, collisionRc;
	uint32_t collisionsNumber = 0, i = 0;
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
		bool isOnLadderTop = collisionRc.bottom < 32;
		if (isPlayer)
			isOnLadderTop = !BasePlaneObject::player->isClimbing() && isOnLadderTop;

		if (isOnLadderTop)
			_onGround(); // ladder top behaves like ground

		if (isPlayer) // let Captain Claw climb
		{
			BasePlaneObject::player->setLadderFlags(isOnLadderTop);
			if (BasePlaneObject::player->isClimbing())
			{
				// set the player position on the ladder easily for the user
				BasePlaneObject::player->position.x = (tileRc.left + tileRc.right) / 2;
			}
		}
	};

	for (const auto& [rect, type] : _rects)
	{
		tileRc = rect;
		collisionRc = objRc.getCollision(tileRc);
		if (!collisionRc.isEmpty())
		{
			switch (type)
			{
			case WwdTileDescription::TileAttribute_Solid: _addCollision(); break;
			case WwdTileDescription::TileAttribute_Ground: _onGround(); break;
			case WwdTileDescription::TileAttribute_Climb: _onLadder(); break;
			case WwdTileDescription::TileAttribute_Death: obj->whenTouchDeath(); break;
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
	Rectangle2D enemyRect(enemyPos.x, enemyPos.y, enemyPos.x, enemyPos.y + 80);

	for (const auto& [rect, type] : _rects)
	{
		if ((type == WwdTileDescription::TileAttribute_Solid || type == WwdTileDescription::TileAttribute_Ground)
			&& rect.intersects(enemyRect))
		{
			float left = rect.left + 32, right = rect.right - 32;

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
