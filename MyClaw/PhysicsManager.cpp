#include "PhysicsManager.h"
#include "Player.h"
#include "WindowManager.h"


#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x += collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define EMPTY_TILE -1


// TODO: add function to update object position

const float PhysicsManager::myGRAVITY = GRAVITY;

#define assert(b) if (!(b)) throw Exception(#b);

Rectangle2D makeRectangle2D(float x, float y, float w, float h)
{
	return Rectangle2D(x, y, x + w, y + h);
}
bool IsInBetween(int32_t num, int32_t leftLimit, int32_t rightLimit)
{
	return ((num > leftLimit) && (num < rightLimit));
}


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
	else if (levelNumber == 4)
	{
		// TODO: delete
	//	WwdTileDescription& t155 = wwd->tilesDescription[155];
	//	t155.rect.top = 0;
	//	cout << 'a';
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
	else if (levelNumber == 7)
	{
		// TODO: tiles (308, 310, 312, 313)
	}
	else if (levelNumber == 8)
	{
		// TODO: tiles (98, 102)
	}
	else if (levelNumber == 11)
	{
		// TODO: tiles (9)
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

		if (attrib == WwdTileDescription::TileAttribute_Clear)
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
#define NEW_CODE
#ifdef NEW_CODE
				// the next code is based on OpenClaw

				// My paper case (1)
				// This yields 2 rectangles
				if ((tileDesc.rect.left == 0 && tileDesc.rect.top == 0) && // Starting in upper left corner
					(tileDesc.rect.right == tileDesc.width - 1)) // Ending on right side of the rect
				{
					// Inside rect
					Rectangle2D rect1 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.width, tileDesc.rect.bottom);
					// Outside rect
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top + tileDesc.rect.bottom + 1, tileDesc.width, tileDesc.height - tileDesc.rect.bottom);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
				}
				// My paper case (2)
				// This yields 2 rectangles
				else if ((tileDesc.rect.left > 0 && tileDesc.rect.top == 0) && // Starting in between right and left corners with top being on the top side
					(tileDesc.rect.right == tileDesc.width - 1) && (tileDesc.rect.bottom == tileDesc.height - 1)) // Ends in bottom right corner of the tile
				{
					// Inside rect
					Rectangle2D rect1 = makeRectangle2D(tileRc.left + tileDesc.rect.left, tileRc.top, tileDesc.width - tileDesc.rect.left, tileDesc.height);
					// Outside rect
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.rect.left, tileDesc.height);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
				}
				// My paper case (3)
				// This yields 2 rectangles
				else if ((tileDesc.rect.left == 0) && (tileDesc.rect.top > 0) && // Starting on left side between top and bottom
					(tileDesc.rect.right == tileDesc.width - 1) && (tileDesc.rect.bottom == tileDesc.height - 1)) // ending in bottom right corner
				{
					// Inside rect
					Rectangle2D rect1 = makeRectangle2D(tileRc.left, tileRc.top + tileDesc.rect.top, tileDesc.width, tileDesc.height - tileDesc.rect.top);
					// Outside rect
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.width, tileDesc.rect.top);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
				}
				// My paper case (4)
				// This yields 2 rectangles
				else if ((tileDesc.rect.left == 0) && (tileDesc.rect.top == 0) && // Starting in upper left corner
					(tileDesc.rect.right > 0) && (tileDesc.rect.bottom == tileDesc.height - 1)) // Ending on the bottom line somewhere between left and right
				{
					// Inside rect
					Rectangle2D rect1 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.rect.right, tileDesc.height);
					// Outside rect
					Rectangle2D rect2 = makeRectangle2D(tileRc.left + tileDesc.rect.right + 1, tileRc.top, tileDesc.width - tileDesc.rect.right, tileDesc.height);

					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
				}
				// My paper case (5)
				// This yields 3 rectangles
				else if ((tileDesc.rect.left == 0) && (tileDesc.rect.top > 0) && // Starting on the left side somewhere between top and bottom
					(tileDesc.rect.right == tileDesc.width - 1) && (tileDesc.rect.bottom > 0)) // !!! Not 100% but it works because we have case (3) ending on right side somewhere between top and bottom
				{
					Rectangle2D rect1 = makeRectangle2D(0, 0, tileDesc.width, tileDesc.rect.top);
					Rectangle2D rect2 = makeRectangle2D(0, 0, tileDesc.width, tileDesc.rect.bottom - tileDesc.rect.top);
					Rectangle2D rect3 = makeRectangle2D(0, tileDesc.rect.bottom, tileDesc.width, tileDesc.height - tileDesc.rect.bottom);
					addRect(rect1, tileDesc.outsideAttrib);
					addRect(rect2, tileDesc.insideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
				}
				// My paper case (6)
				// This yields 3 rectangles
				else if ((tileDesc.rect.left > 0) && (tileDesc.rect.top == 0) && // Starting on top side somewhere between left and right
					(tileDesc.rect.right > 0) && (tileDesc.rect.bottom == tileDesc.height - 1)) // !!! similiar to (2)
				{
					Rectangle2D rect1 = makeRectangle2D(0, 0, tileDesc.rect.left, tileDesc.height);
					Rectangle2D rect2 = makeRectangle2D(tileDesc.rect.left, 0, tileDesc.rect.right - tileDesc.rect.left, tileDesc.height);
					Rectangle2D rect3 = makeRectangle2D(tileDesc.rect.right, 0, tileDesc.width - tileDesc.rect.right, tileDesc.height);
					addRect(rect1, tileDesc.outsideAttrib);
					addRect(rect2, tileDesc.insideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
				}
				// My paper case (7) --- PAPERFIED
				// This yields 3 rectangles
				else if ((tileDesc.rect.left == 0) && (tileDesc.rect.top == 0) && // Starting in top left corner
					(tileDesc.rect.right != tileDesc.width - 1) && (tileDesc.rect.bottom != tileDesc.height - 1)) // Ending somewhere inside the rect
				{
					Rectangle2D rect1 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.rect.right, tileDesc.rect.bottom);
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top + tileDesc.rect.bottom + 1, tileDesc.rect.right + 1, tileDesc.height - tileDesc.rect.bottom);
					Rectangle2D rect3 = makeRectangle2D(tileRc.left + tileDesc.rect.right + 1, tileRc.top, tileDesc.width - tileDesc.rect.right, tileDesc.height);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
				}
				// My paper case (8) --- PAPERFIED
				// This yields 3 rectangles
				else if ((tileDesc.rect.left > 0) && (tileDesc.rect.top == 0) && // Starting somewhere on the top between left and right
					(tileDesc.rect.right == tileDesc.width - 1) && (tileDesc.rect.bottom != tileDesc.height - 1)) // ending somewhere on the right side between top and bottom
				{
					Rectangle2D rect1 = makeRectangle2D(tileRc.left + tileDesc.rect.left, tileRc.top, tileDesc.width - tileDesc.rect.left, tileDesc.rect.bottom);
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.rect.left - 1, tileDesc.height);
					Rectangle2D rect3 = makeRectangle2D(tileRc.left + tileDesc.rect.left, tileRc.top + tileDesc.rect.bottom + 1, tileDesc.width - tileDesc.rect.left, tileDesc.height - tileDesc.rect.bottom - 1);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
				}
				// My paper case (9) --- PAPERFIED
				// This yields 3 rectangles
				else if ((tileDesc.rect.left > 0) && (tileDesc.rect.top > 0) &&
					(tileDesc.rect.right == tileDesc.width - 1) && (tileDesc.rect.bottom == tileDesc.height - 1))
				{
					Rectangle2D rect1 = makeRectangle2D(tileRc.left + tileDesc.rect.left, tileRc.top + tileDesc.rect.top, tileDesc.width - tileDesc.rect.left, tileDesc.height - tileDesc.rect.top);
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.rect.left, tileDesc.height);
					Rectangle2D rect3 = makeRectangle2D(tileRc.left + tileDesc.rect.left, tileRc.top, tileDesc.width - tileDesc.rect.left, tileDesc.rect.top);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
				}
				// My paper case (10) --- PAPERFIED
				// This yields 3 rectangles
				else if ((tileDesc.rect.left == 0) && (tileDesc.rect.top > 0) && // Beginning somewhere on the left side between top and bottom
					(tileDesc.rect.right != tileDesc.width - 1) && (tileDesc.rect.bottom == tileDesc.height - 1))
				{
					Rectangle2D rect1 = makeRectangle2D(tileRc.left, tileRc.top + tileDesc.rect.top, tileDesc.rect.right, tileDesc.height - tileDesc.rect.top);
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.rect.right, tileDesc.rect.top);
					Rectangle2D rect3 = makeRectangle2D(tileRc.left + tileDesc.rect.right, tileRc.top, tileDesc.width - tileDesc.rect.right, tileDesc.height);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
				}
				// My paper case (11)
				// This yields 4 rectangles
				else if (IsInBetween(tileDesc.rect.left, 0, tileDesc.width - 1) && (tileDesc.rect.top == 0) &&
					(IsInBetween(tileDesc.rect.right, 0, tileDesc.width - 1)) && (IsInBetween(tileDesc.rect.bottom, 0, tileDesc.height - 1)))
				{
					Rectangle2D rect1 = makeRectangle2D(tileDesc.rect.left, tileDesc.rect.top, tileDesc.rect.right - tileDesc.rect.left, tileDesc.rect.bottom);
					addRect(rect1, tileDesc.insideAttrib);
					assert(tileDesc.insideAttrib != WwdTileDescription::TileAttribute_Clear && tileDesc.outsideAttrib == WwdTileDescription::TileAttribute_Clear);
				}
				// My paper case (12)
				// This yields 4 rectangles
				else if (IsInBetween(tileDesc.rect.left, 0, tileDesc.width - 1) && IsInBetween(tileDesc.rect.top, 0, tileDesc.height - 1) &&
					(tileDesc.rect.right == tileDesc.width - 1) && IsInBetween(tileDesc.rect.bottom, 0, tileDesc.height - 1))
				{
					//	cout << "Case 12: Tile " << plane->tiles[i][j] << " not implemented !" << endl;

					Rectangle2D rect1 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.width, tileDesc.rect.top);
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top + tileDesc.rect.top, tileDesc.rect.left, tileDesc.rect.bottom - tileDesc.rect.top);
					Rectangle2D rect4 = makeRectangle2D(tileRc.left + tileDesc.rect.left + 1, tileRc.top + tileDesc.rect.top, tileDesc.width - tileDesc.rect.left, tileDesc.rect.bottom - tileDesc.rect.top);
					Rectangle2D rect3 = makeRectangle2D(tileRc.left, tileRc.top + tileDesc.rect.bottom + 1, tileDesc.width, tileDesc.height - tileDesc.rect.bottom);

					addRect(rect1, tileDesc.outsideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
					addRect(rect4, tileDesc.insideAttrib);
				}
				// My paper case (13) --- PAPERFIED
				// This yields 4 rectangles
				else if (IsInBetween(tileDesc.rect.left, 0, tileDesc.width - 1) && IsInBetween(tileDesc.rect.top, 0, tileDesc.height - 1) &&
					IsInBetween(tileDesc.rect.right, 0, tileDesc.width - 1) && (tileDesc.rect.bottom == tileDesc.height - 1))
				{
					Rectangle2D rect1 = makeRectangle2D(tileRc.left + tileDesc.rect.left, tileRc.top + tileDesc.rect.top, tileDesc.rect.right - tileDesc.rect.left, tileDesc.height - tileDesc.rect.top);
					Rectangle2D rect2 = makeRectangle2D(tileRc.left, tileRc.top, tileDesc.rect.left, tileDesc.height);
					Rectangle2D rect3 = makeRectangle2D(tileRc.left + tileDesc.rect.left, tileRc.top, tileDesc.rect.right - tileDesc.rect.left, tileDesc.rect.top);
					Rectangle2D rect4 = makeRectangle2D(tileRc.left + tileDesc.rect.right, tileRc.top, tileDesc.width - tileDesc.rect.right, tileDesc.height);
					addRect(rect1, tileDesc.insideAttrib);
					addRect(rect2, tileDesc.outsideAttrib);
					addRect(rect3, tileDesc.outsideAttrib);
					addRect(rect4, tileDesc.outsideAttrib);
				}
				// My paper case (14)
				// This yields 4 rectangles
				else if ((tileDesc.rect.left == 0) && IsInBetween(tileDesc.rect.top, 0, tileDesc.height - 1) &&
					IsInBetween(tileDesc.rect.right, 0, tileDesc.width - 1) && (tileDesc.rect.bottom == tileDesc.height - 1))
				{
					cout << "Paring rects case: 14" << endl;
					assert(false && "Paring rects case: 14");
				}
				// My paper case (15)
				// This yields 5 rectangles
				else if (IsInBetween(tileDesc.rect.left, 0, tileDesc.width - 1) && IsInBetween(tileDesc.rect.top, 0, tileDesc.height - 1) &&
					IsInBetween(tileDesc.rect.right, 0, tileDesc.width - 1) && (tileDesc.rect.bottom == tileDesc.height - 1))
				{
					cout << "Paring rects case: 16" << endl;
					assert(false && "Paring rects case: 16");
				}
				else
				{
					cout << "unknown tile" << endl;
				}


#else
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
					// TODO: split to rect for inAttr and outAttr
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
#endif
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
