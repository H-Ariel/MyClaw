#include "LevelPlane.h"
#include "GameEngine/WindowManager.h"


LevelPlane::LevelPlane(WapWwd* wwd, WwdPlane* wwdPlane)
	: _wwd(wwd), _wwdPlane(wwdPlane), _isMainPlane(false),
	maxTileIdxX(((wwdPlane->flags& WwdPlane::WwdPlaneFlags_XWrapping) ? INT32_MAX : wwdPlane->tilesOnAxisX)),
	maxTileIdxY(((wwdPlane->flags& WwdPlane::WwdPlaneFlags_YWrapping) ? INT32_MAX : wwdPlane->tilesOnAxisY))
{
}

void LevelPlane::Draw()
{
	shared_ptr<UIBaseImage> img;
	size_t i;
	int row, col, tileId, rowTileIndex;

	const D2D1_SIZE_F wndSz = WindowManager::getSize();
	const float parallaxCameraPosX = position.x * _wwdPlane->movementPercentX;
	const float parallaxCameraPosY = position.y * _wwdPlane->movementPercentY;
	const int startRow = int(parallaxCameraPosY / TILE_SIZE);
	const int startCol = int(parallaxCameraPosX / TILE_SIZE);
	const int endRow = min<int>(maxTileIdxY, int(wndSz.height / TILE_SIZE + 2 + startRow));
	const int endCol = min<int>(maxTileIdxX, int(wndSz.width / TILE_SIZE + 2 + startCol));

	for (i = 0; i < _objects.size() && _objects[i]->drawZ < _wwdPlane->coordZ; i++)
		_objects[i]->Draw();

	for (row = startRow; row < endRow; row++)
	{
		rowTileIndex = row % _wwdPlane->tilesOnAxisY;
		for (col = startCol; col < endCol; col++)
		{
			tileId = _wwdPlane->tiles[rowTileIndex][col % _wwdPlane->tilesOnAxisX];
			if (tilesImages.count(tileId))
			{
				img = tilesImages.at(tileId);
				img->position.x = (col + 0.5f) * TILE_SIZE - parallaxCameraPosX + position.x;
				img->position.y = (row + 0.5f) * TILE_SIZE - parallaxCameraPosY + position.y;
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

	DBG_PRINT("WARNING: The AmbientSound objects are too loud so I decided to not play it\n");

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
			DBG_PRINT("Error while reading object: %s\n", ex.message.c_str());
		}
	}

	_wwdPlane->objects.clear();
}

// update objects data after reading them from the file so it be easier to use them
void LevelPlane::updateObject(WwdObject& obj)
{
	if (obj.logic == "ConveyorBelt")
	{
		int32_t x = obj.x - obj.x % TILE_SIZE, y = obj.y - obj.y % TILE_SIZE;
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
