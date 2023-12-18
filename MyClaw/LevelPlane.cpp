#include "LevelPlane.h"
#include "GUI/WindowManager.h"


LevelPlane::LevelPlane(WapWorld* wwd)
	: _wwd(wwd), fillColor(0), tilesOnAxisX(0), tilesOnAxisY(0),
	maxTileIdxX(0), maxTileIdxY(0), ZCoord(0), movementPercentX(0),
	movementPercentY(0), _isMainPlane(false)
{
}

void LevelPlane::Draw()
{
	shared_ptr<UIBaseImage> img;
	size_t i;
	int row, col, tileId, rowTileIndex;

	const D2D1_SIZE_F wndSz = WindowManager::getSize();
	const float parallaxCameraPosX = position.x * movementPercentX;
	const float parallaxCameraPosY = position.y * movementPercentY;
	const int startRow = int(parallaxCameraPosY / TILE_SIZE);
	const int startCol = int(parallaxCameraPosX / TILE_SIZE);
	const int endRow = min<int>(maxTileIdxY, int(wndSz.height / TILE_SIZE + 2 + startRow));
	const int endCol = min<int>(maxTileIdxX, int(wndSz.width / TILE_SIZE + 2 + startCol));

	for (i = 0; i < _objects.size() && _objects[i]->drawZ < this->ZCoord; i++)
		_objects[i]->Draw();

	for (row = startRow; row < endRow; row++)
	{
		rowTileIndex = row % tilesOnAxisY;
		for (col = startCol; col < endCol; col++)
		{
			tileId = tiles[rowTileIndex][col % tilesOnAxisX];
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

void LevelPlane::readPlaneObjects(BufferReader& reader, int numOfObjects)
{
	WwdObject obj;
	uint32_t nameLength, logicLength, imageSetLength, animationLength;
	
	while (numOfObjects--)
	{
		reader.read(obj.id);
		reader.read(nameLength);
		reader.read(logicLength);
		reader.read(imageSetLength);
		reader.read(animationLength);
		reader.read(obj.x);
		reader.read(obj.y);
		reader.read(obj.z);
		reader.read(obj.i);
		reader.read(obj.addFlags);
		reader.read(obj.dynamicFlags);
		reader.read(obj.drawFlags);
		reader.read(obj.userFlags);
		reader.read(obj.score);
		reader.read(obj.points);
		reader.read(obj.powerup);
		reader.read(obj.damage);
		reader.read(obj.smarts);
		reader.read(obj.health);
		reader.read(obj.moveRect);
		reader.read(obj.hitRect);
		reader.read(obj.attackRect);
		reader.read(obj.clipRect);
		reader.read(obj.userRect1);
		reader.read(obj.userRect2);
		reader.read(obj.userValue1);
		reader.read(obj.userValue2);
		reader.read(obj.userValue3);
		reader.read(obj.userValue4);
		reader.read(obj.userValue5);
		reader.read(obj.userValue6);
		reader.read(obj.userValue7);
		reader.read(obj.userValue8);
		reader.read(obj.minX);
		reader.read(obj.minY);
		reader.read(obj.maxX);
		reader.read(obj.maxY);
		reader.read(obj.speedX);
		reader.read(obj.speedY);
		reader.read(obj.tweakX);
		reader.read(obj.tweakY);
		reader.read(obj.counter);
		reader.read(obj.speed);
		reader.read(obj.width);
		reader.read(obj.height);
		reader.read(obj.direction);
		reader.read(obj.faceDir);
		reader.read(obj.timeDelay);
		reader.read(obj.frameDelay);
		reader.read(obj.objectType);
		reader.read(obj.hitTypeFlags);
		reader.read(obj.moveResX);
		reader.read(obj.moveResY);
		obj.name = reader.ReadString(nameLength);
		obj.logic = reader.ReadString(logicLength);
		obj.imageSet = reader.ReadString(imageSetLength);
		obj.animation = reader.ReadString(animationLength);
		
		try
		{
			if (endsWith(obj.logic, "AmbientSound"))
			{
				DBG_PRINT("The AmbientSound objects are too loud so I decided to not play it\n");
				continue;
			}

			updateObject(obj);
			addObject(obj);
		}
		catch (const Exception& ex)
		{
			DBG_PRINT("Error while reading object: %s\n", ex.what().c_str());
		}
	}
}

// update objects data after reading them from the file so it be easier to use them
void LevelPlane::updateObject(WwdObject& obj)
{
	if (obj.logic == "ConveyorBelt")
	{
		int32_t x = obj.x - obj.x % TILE_SIZE, y = obj.y - obj.y % TILE_SIZE;
		obj.moveRect = _wwd->tilesDescription[tiles[obj.y / TILE_SIZE][obj.x / TILE_SIZE]].rect;
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

void LevelPlane::addObject(const WwdObject& obj) {}
