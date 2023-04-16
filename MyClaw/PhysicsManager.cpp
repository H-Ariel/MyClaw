#include "PhysicsManager.h"
#include "Player.h"
#include "WindowManager.h"


#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x += collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define EMPTY_TILE -1


// TODO: add function to update object position

/*
VERY IMPORTANT TODO: think if the rects-lists idea is realy better
than the previues code

IMPORTANT TODO: (this is the current mission of this class)
svae rects with type, and check only the rect who collide with the object
*/


const float PhysicsManager::myGRAVITY = GRAVITY;

PhysicsManager::PhysicsManager(const WwdPlane& plane, const shared_ptr<WapWorld>& wwd, Player*& player)
	: _plane(plane), _wwd(wwd), _player(player)
{
}

void PhysicsManager::reduceRectangles()
{
	size_t i, j;

	// this loop combines rectangles that are next to each other (according to x axis)
	for (i = 0; i < _rects.size(); i++)
	{
		for (j = i + 1; j < _rects.size(); j++)
		{
			if (_rects[i].second == _rects[j].second &&
				_rects[i].first.top == _rects[j].first.top &&
				_rects[i].first.bottom == _rects[j].first.bottom &&
				_rects[i].first.right == _rects[j].first.left)
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
				_rects[i].first.left == _rects[j].first.left &&
				_rects[i].first.right == _rects[j].first.right &&
				_rects[i].first.bottom == _rects[j].first.top)
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

void PhysicsManager::init(int8_t levelNumber)
{
	// map of all rectangles that player can collide with 

	Rectangle2D tileRc, originalTileRc, rc1, rc2;
	uint32_t i, j;



	// minor change to tiles so they will be more accurate for `reduceRectangles`
	// TODO: continue for all levels
	if (levelNumber == 1)
	{
		WwdTileDescription& t401 = _wwd->tilesDescription[401];
		WwdTileDescription& t406 = _wwd->tilesDescription[406];

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
				if (tileDesc.insideAttrib != WwdTileDescription::TileAttribute_Clear)
				{
					_rects.push_back({ tileRc, tileDesc.insideAttrib });
				}
			}	break;

			case WwdTileDescription::TileType_Double: { // TODO: improve this part
				originalTileRc = tileRc;

				tileRc.left += tileDesc.rect.left;
				tileRc.top += tileDesc.rect.top;
				tileRc.right = tileRc.right + tileDesc.rect.right - tileDesc.width + 1;
				tileRc.bottom = tileRc.bottom + tileDesc.rect.bottom - tileDesc.height + 1;

				if (tileDesc.insideAttrib != WwdTileDescription::TileAttribute_Clear)
				{
					_rects.push_back({ tileRc, tileDesc.insideAttrib });
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

					_rects.push_back({ rc1, tileDesc.outsideAttrib });
					_rects.push_back({ rc2, tileDesc.outsideAttrib });
				}
			}	break;

			default: break;
			}
		}
	}

	cout << "before: " << _rects.size() << endl;
	reduceRectangles();
	cout << "after: " << _rects.size() << endl;
}

void PhysicsManager::Draw()
{
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
}

void PhysicsManager::checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath)
{
	static const float N = 1.5f; // this number indicate how many tile we will check from every side
	const Rectangle2D objRc = obj->GetRect();
	const uint32_t MinXPos = (uint32_t)(objRc.left / _plane.tilePixelWidth - N);
	const uint32_t MaxXPos = (uint32_t)(objRc.right / _plane.tilePixelWidth + N);
	const uint32_t MinYPos = (uint32_t)(objRc.top / _plane.tilePixelHeight - N);
	const uint32_t MaxYPos = (uint32_t)(objRc.bottom / _plane.tilePixelHeight + N);

	Rectangle2D collisions[9];
	Rectangle2D cumulatedCollision, tileRc;
	Rectangle2D collisionRc;
	Rectangle2D originalTileRc, rc1, rc2;
	uint32_t i = 0, j = 0, collisionsNumber = 0;

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
			/*
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
			*/

			// check if object is at the top of the ladder, so it should stay here (and not fall)
			bool isOnLadderTop = objRc.bottom - tileRc.top < 32;
			if (isPlayer)
				isOnLadderTop = !_player->isClimbing() && isOnLadderTop;
			///////////////////////////////////

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


	// TODO: use (?) _rects (the `else` block)
	if (!true) // (delete this condition)
	{
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
	}
	else
	{
		for (auto& p : _rects)
		{
			tileRc = p.first;
			if (objRc.intersects(tileRc))
			{
				switch (p.second)
				{
				case WwdTileDescription::TileAttribute_Solid: _onSolid(tileRc); break;
				case WwdTileDescription::TileAttribute_Ground: _onGround(); break;
				case WwdTileDescription::TileAttribute_Climb: _onLadder(); break;
				case WwdTileDescription::TileAttribute_Death: _onDeath(); break;
				default: break;
				}
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
