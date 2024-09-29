#include "LevelPlane.h"
#include "GameEngine/WindowManager.h"
#include "SavedDataManager.h"


const float STEP_SIZE = TILE_SIZE - 0.6f; // small delta so that there is no space between tiles


LevelPlane::LevelPlane(WapWwd* wwd, WwdPlane* wwdPlane)
	: _wwd(wwd), _wwdPlane(wwdPlane), isVisible(true),
	maxTileIdxX(((wwdPlane->flags & WwdPlane::WwdPlaneFlags_XWrapping) ? INT_MAX : wwdPlane->tilesOnAxisX)),
	maxTileIdxY(((wwdPlane->flags & WwdPlane::WwdPlaneFlags_YWrapping) ? INT_MAX : wwdPlane->tilesOnAxisY))
{
}

void LevelPlane::Draw()
{
	if (!isVisible) return;

	shared_ptr<UIBaseImage> img;
	size_t i;
	int row, col, tileId, rowTileIndex, colTileIndex;
	float x, y;

	const D2D1_SIZE_F camSz = WindowManager::getCameraSize();
	const float parallaxCameraPosX = position.x * _wwdPlane->movementPercentX;
	const float parallaxCameraPosY = position.y * _wwdPlane->movementPercentY;
	const int startRow = int(parallaxCameraPosY / TILE_SIZE);
	const int startCol = max(int(parallaxCameraPosX / TILE_SIZE) - 1, 0);
	const int endRow = min(maxTileIdxY, int(camSz.height / TILE_SIZE + 2 + startRow));
	const int endCol = min(maxTileIdxX, int(camSz.width / TILE_SIZE + 3 + startCol));
	const float startY = (startRow + 0.5f) * TILE_SIZE - parallaxCameraPosY + position.y;
	const float startX = (startCol + 0.5f) * TILE_SIZE - parallaxCameraPosX + position.x;

	for (i = 0; i < _objects.size() && _objects[i]->drawZ < _wwdPlane->coordZ; i++)
		_objects[i]->Draw();

	for (row = startRow, y = startY; row < endRow; row++, y += STEP_SIZE)
	{
		rowTileIndex = row % _wwdPlane->tilesOnAxisY;

		for (col = startCol, x = startX; col < endCol; col++, x += STEP_SIZE)
		{
			colTileIndex = col % _wwdPlane->tilesOnAxisX;
			tileId = _wwdPlane->tiles[rowTileIndex][colTileIndex];
			if (tilesImages.count(tileId))
			{
				img = tilesImages.at(tileId);
				img->position.x = x;
				img->position.y = y;
				img->Draw();

				/*
#ifdef _DEBUG
				// draw black rectangle around the tile
				if (isMainPlane())
					WindowManager::drawRect(img->GetRect(), ColorF::Black);
#endif
				*/
			}
		}
	}

	for (; i < _objects.size(); i++)
		_objects[i]->Draw();
}

void LevelPlane::init()
{
	tilesImages = AssetsManager::loadPlaneTilesImages(_wwd->imageDirectoryPath + '/' + _wwdPlane->imageSets[0]);

	LogFile::log(LogFile::Warning, "The AmbientSound objects are too loud so I decided to not play it");

	for (WwdObject& obj : _wwdPlane->objects)
	{
		try
		{
			if (endsWith(obj.logic, "AmbientSound"))
			{
				continue;
			}

			updateObject(obj);
			addObject(obj);
		}
		catch (const Exception& ex)
		{
			LogFile::log(LogFile::Error, "while reading object: %s", ex.message.c_str());
		}
	}

	_wwdPlane->objects.clear();
}

// update objects data after reading them from the file so it be easier to use them
void LevelPlane::updateObject(WwdObject& obj)
{
	if (obj.logic == "ConveyorBelt")
	{
		int x = obj.x - obj.x % TILE_SIZE, y = obj.y - obj.y % TILE_SIZE;
		obj.moveRect = _wwd->tileDescriptions[_wwdPlane->tiles[obj.y / TILE_SIZE][obj.x / TILE_SIZE]].rect;
		obj.moveRect.left += x;
		obj.moveRect.right += x;
		obj.moveRect.top += y;
		obj.moveRect.bottom += y;
	}
	else if (obj.logic == "TigerGuard")
	{
		if (obj.smarts == 1)
			obj.imageSet += "WHITE";
	}
	else if (obj.logic == "AquatisDynamite")
	{
		obj.damage = 1; // respawn object
	}
}

// add object to the plane
void LevelPlane::addObject(const WwdObject& obj) {}
