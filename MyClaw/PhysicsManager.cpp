#include "PhysicsManager.h"
#include "Player.h"
#include "WindowManager.h"


#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x += collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define EMPTY_TILE -1


// TODO: add function to update object position

const float PhysicsManager::myGRAVITY = GRAVITY;

// TODO: delete this ?
// if stay move to "framework.h"
#define assert(b) if (!(b)){\
	char buf[256]; sprintf(buf, "%d [%d]: %s", __LINE__, plane->tiles[i][j], #b);\
	throw Exception(buf); }


PhysicsManager::PhysicsManager(const WwdPlaneData * plane, WapWorld * wwd, Player * player, int levelNumber)
	: _player(player)
{
	// map of all rectangles that BaseDynamicPlaneObjects can collide with 

	Rectangle2D tileRc, originalTileRc;
	uint32_t i, j;

	// add rectangle to list and merge it with previous rectangle if possible
	auto addRect = [&](const Rectangle2D& rc, uint32_t attrib) {

		if (attrib == WwdTileDescription::TileAttribute_Clear ||
			rc.left == rc.right || rc.top == rc.bottom)
			return;

		pair<Rectangle2D, uint32_t> curr = { rc, attrib };

		if (_rects.empty())
		{
			_rects.push_back(curr);
			return;
		}

		pair<Rectangle2D, uint32_t>& last = _rects.back();

		if (last.second == curr.second &&
			abs(last.first.top - curr.first.top) <= 2 &&
			abs(last.first.bottom - curr.first.bottom) <= 2 &&
			abs(last.first.right - curr.first.left) <= 2)
		{
			Rectangle2D newRc(last.first.left, last.first.top,
				curr.first.right, last.first.bottom);
			last.first = newRc;
		}
		else
		{
			_rects.push_back(curr);
		}
	};

	float x1, x2, y1, y2;

	for (i = 0; i < plane->tilesOnAxisY; i++)
	{
		for (j = 0; j < plane->tilesOnAxisX; j++)
		{
			const WwdTileDescription& tileDesc = wwd->tilesDescription[plane->tiles[i][j]];

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

			default: break;
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
				Rectangle2D newRc(_rects[i].first.left, _rects[i].first.top,
					_rects[j].first.right, _rects[i].first.bottom);
				_rects.erase(_rects.begin() + j);
				_rects[i].first = newRc;
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
				Rectangle2D newRc(_rects[i].first.left, _rects[i].first.top,
					_rects[i].first.right, _rects[j].first.bottom);
				_rects.erase(_rects.begin() + j);
				_rects[i].first = newRc;
				j--;
			}
		}
	}
}

void PhysicsManager::Draw()
{
#ifdef _DEBUG
	ColorF clr(0);
	for (auto& p : _rects)
	{
		switch (p.second)
		{
		case WwdTileDescription::TileAttribute_Solid: clr = ColorF::Red; break;
		case WwdTileDescription::TileAttribute_Ground: clr = ColorF::Magenta; break;
		case WwdTileDescription::TileAttribute_Climb: clr = ColorF::Green; break;
		case WwdTileDescription::TileAttribute_Death: clr = ColorF::Blue; break;
		default: clr = ColorF(0, 0);
		}
		WindowManager::drawRect(p.first, clr);
	}
#endif
}

void PhysicsManager::checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath)
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
	auto _onGround = [&]() { // same to `BasePlaneObject::tryCatchPlayer`
		if (obj->isFalling() && (collisionRc.right > 0 || collisionRc.left > 0) && collisionRc.getSmallest().bottom > 0)
		{
			_addCollision();
		}
	};
	auto _onLadder = [&]() {
		// check if object is at the top of the ladder, so it should stay here (and not fall)
		bool isOnLadderTop = collisionRc.bottom < 32;
		if (isPlayer)
			isOnLadderTop = !_player->isClimbing() && isOnLadderTop;

		if (isOnLadderTop)
			_onGround(); // ladder top behaves like ground

		if (isPlayer) // let Captain Claw climb
		{
			_player->setLadderFlags(isOnLadderTop);
			if (_player->isClimbing())
			{
				// set the player position on the ladder easily for the user
				_player->position.x = (tileRc.left + tileRc.right) / 2;
			}
		}
	};

	for (auto& p : _rects)
	{
		if (objRc.intersects(p.first))
		{
			tileRc = p.first;
			collisionRc = objRc.getCollision(tileRc);
			switch (p.second)
			{
			case WwdTileDescription::TileAttribute_Solid: _addCollision(); break;
			case WwdTileDescription::TileAttribute_Ground: _onGround(); break;
			case WwdTileDescription::TileAttribute_Climb: _onLadder(); break;
			case WwdTileDescription::TileAttribute_Death: whenTouchDeath(); break;
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

	for (auto& p : _rects)
	{
		if (p.first.intersects(enemyRect) && (
			p.second == WwdTileDescription::TileAttribute_Solid ||
			p.second == WwdTileDescription::TileAttribute_Ground))
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
