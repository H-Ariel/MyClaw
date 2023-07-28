#include "LevelPlane.h"
#include "WindowManager.h"


LevelPlane::LevelPlane()
	: fillColor(0), tilesOnAxisX(0), tilesOnAxisY(0), maxTileIdxX(0), maxTileIdxY(0),
	ZCoord(0), movementPercentX(0), movementPercentY(0), _isMainPlane(false)
{
}

void LevelPlane::init() {}

void LevelPlane::Draw()
{
	shared_ptr<UIBaseImage> img;
	int row, col, tileId, rowTileIndex;

	const D2D1_SIZE_F wndSz = WindowManager::getSize();
	const float parallaxCameraPosX = position.x * movementPercentX;
	const float parallaxCameraPosY = position.y * movementPercentY;
	const int startRow = int(parallaxCameraPosY / TILE_SIZE);
	const int startCol = int(parallaxCameraPosX / TILE_SIZE);
	const int endRow = min<int>(maxTileIdxY, int(wndSz.height / TILE_SIZE + 2 + startRow));
	const int endCol = min<int>(maxTileIdxX, int(wndSz.width / TILE_SIZE + 2 + startCol));

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
			}
		}
	}
}

void LevelPlane::readPlaneObjects(BufferReader& reader)
{
	uint32_t nameLength, logicLength, imageSetLength, animationLength;

	for (WwdObject& obj : objects)
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
	}

	sort(objects.begin(), objects.end(), [](const WwdObject& a, const WwdObject& b) { return a.z < b.z; });
}
