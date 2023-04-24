#include "PhysicsManager.h"
#include "Player.h"
#include "WindowManager.h"


#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x += collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define EMPTY_TILE -1


// TODO: add function to update object position

const float PhysicsManager::myGRAVITY = GRAVITY;


PhysicsManager::PhysicsManager(const WwdPlaneData* plane, WapWorld* wwd, Player* player, int levelNumber)
	: _player(player)
{
	// map of all rectangles that BaseDynamicPlaneObjects can collide with 

	Rectangle2D tileRc, originalTileRc;
	uint32_t i, j;

	// minor change to tiles so they will be more accurate for reduce rectangles
	// (in levels 5,11 it is necessary to change the tiles for "BreakPlanks")
	// TODO: continue for all levels
	// TODO: move to `WwdFile`
	if (levelNumber == 1)
	{
		WwdTileDescription& t401 = wwd->tilesDescription[401];
		WwdTileDescription& t406 = wwd->tilesDescription[406];

		t401.rect.right = 28;
		t401.rect.bottom = 63;
		t401.insideAttrib = WwdTileDescription::TileAttribute_Solid;
		t401.type = WwdTileDescription::TileType_Double;

		t406.rect.left = 40;
		t406.rect.right = 63;
		t406.rect.bottom = 63;
		t406.insideAttrib = WwdTileDescription::TileAttribute_Solid;
		t406.type = WwdTileDescription::TileType_Double;
	}
	else if (levelNumber == 2)
	{
		WwdTileDescription& t403 = wwd->tilesDescription[403];
		t403.insideAttrib = WwdTileDescription::TileAttribute_Solid;
	}
	else if (levelNumber == 5)
	{
		// i think there is something wrong here...
		WwdTileDescription& t407 = wwd->tilesDescription[407];
		WwdTileDescription& t509 = wwd->tilesDescription[509];

		t407.insideAttrib = WwdTileDescription::TileAttribute_Solid;
		t509.insideAttrib = WwdTileDescription::TileAttribute_Clear;
		t509.outsideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 11)
	{
		WwdTileDescription& t39 = wwd->tilesDescription[39];
		t39.insideAttrib = WwdTileDescription::TileAttribute_Clear;
		t39.outsideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 14)
	{
		WwdTileDescription& t049 = wwd->tilesDescription[49];
		WwdTileDescription& t050 = wwd->tilesDescription[50];
		WwdTileDescription& t054 = wwd->tilesDescription[54];

		t049.insideAttrib = WwdTileDescription::TileAttribute_Clear;
		t050.insideAttrib = WwdTileDescription::TileAttribute_Clear;
		t054.insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}

	// add rectangle to list and merge it with previous rectangle if possible
	auto addRect = [&](const Rectangle2D& rc, uint32_t attrib) {
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

	for (i = 0; i < plane->tilesOnAxisY; i++)
	{
		for (j = 0; j < plane->tilesOnAxisX; j++)
		{
			const WwdTileDescription& tileDesc = wwd->tilesDescription[plane->tiles[i][j]];

			tileRc.left = (float)(j * plane->tilePixelWidth);
			tileRc.top = (float)(i * plane->tilePixelHeight);
			tileRc.right = tileRc.left + plane->tilePixelWidth;
			tileRc.bottom = tileRc.top + plane->tilePixelHeight;

			switch (tileDesc.type)
			{
			case WwdTileDescription::TileType_Single:
				if (tileDesc.insideAttrib != WwdTileDescription::TileAttribute_Clear)
					addRect(tileRc, tileDesc.insideAttrib);
				break;

			case WwdTileDescription::TileType_Double: // TODO: improve this part
				originalTileRc = tileRc;

				tileRc.left += tileDesc.rect.left;
				tileRc.top += tileDesc.rect.top;
				tileRc.right = tileRc.right + tileDesc.rect.right - tileDesc.width;
				tileRc.bottom = tileRc.bottom + tileDesc.rect.bottom - tileDesc.height;

				if (tileDesc.insideAttrib != WwdTileDescription::TileAttribute_Clear)
				{
					addRect(tileRc, tileDesc.insideAttrib);
				}

				if (tileDesc.outsideAttrib == WwdTileDescription::TileAttribute_Solid)
				{
					// TODO: use OpenClaw for this part (change my code)

					if (tileDesc.rect.right == tileDesc.width - 1 && tileDesc.rect.top == 0)
					{
						addRect(Rectangle2D(originalTileRc.left, originalTileRc.top, tileRc.left, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(tileRc.left, tileRc.bottom, tileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
					}
					else if (tileDesc.rect.left == 0 && tileDesc.rect.top == 0)
					{
						addRect(Rectangle2D(tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(originalTileRc.left, tileRc.bottom, originalTileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
					}
					else if (tileDesc.rect.right == tileDesc.width - 1 && tileDesc.rect.bottom == tileDesc.height - 1)
					{
						addRect(Rectangle2D(originalTileRc.left, originalTileRc.top, tileRc.left, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(tileRc.left, originalTileRc.top, tileRc.right, tileRc.top), tileDesc.outsideAttrib);
					}
					else if (tileDesc.rect.left == 0 && tileDesc.rect.bottom == tileDesc.height - 1)
					{
						addRect(Rectangle2D(tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(originalTileRc.left, originalTileRc.top, originalTileRc.right, tileRc.top), tileDesc.outsideAttrib);
					}
					/*
					else if (tileDesc.rect.top == 0)
					{
						addRect(Rectangle2D(originalTileRc.left, originalTileRc.top, tileRc.left, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(tileRc.left, tileRc.bottom, tileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
					}
					else if (tileDesc.rect.bottom == tileDesc.height - 1)
					{
						addRect(Rectangle2D(originalTileRc.left, originalTileRc.top, tileRc.left, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(tileRc.left, originalTileRc.top, tileRc.right, tileRc.top), tileDesc.outsideAttrib);
					}
					else if (tileDesc.rect.left == 0)
					{
						addRect(Rectangle2D(tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
						addRect(Rectangle2D(originalTileRc.left, originalTileRc.top, originalTileRc.right, tileRc.top), tileDesc.outsideAttrib);
						addRect(Rectangle2D(originalTileRc.left, tileRc.bottom, originalTileRc.right, originalTileRc.bottom), tileDesc.outsideAttrib);
					}
					*/
					else
					{
						continue;
						// TODO: handle the other case
					}
				}
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
