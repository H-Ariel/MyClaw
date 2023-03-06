#include "LevelPlane.h"
#include "UIBaseImage.h"
#include "WindowManager.h"


LevelPlane::LevelPlane(const WwdPlane& plane)
	: _plane(plane),
	maxTileIdxX(plane.isWrappedX ? INT32_MAX : plane.tilesOnAxisX),
	maxTileIdxY(plane.isWrappedY ? INT32_MAX : plane.tilesOnAxisY)
{
}

void LevelPlane::Draw()
{
	shared_ptr<UIBaseImage> img;
	int32_t tileId, rowTileIndex, colTileIndex;
	uint32_t row, col;

	const D2D1_SIZE_F wndSz = WindowManager::getSize();
	const float parallaxCameraPosX = position.x * _plane.movementPercentX;
	const float parallaxCameraPosY = position.y * _plane.movementPercentY;
	const uint32_t startCol = (uint32_t)(parallaxCameraPosX / _plane.tilePixelWidth);
	const uint32_t startRow = (uint32_t)(parallaxCameraPosY / _plane.tilePixelHeight);
	const uint32_t endRow = min<uint32_t>(maxTileIdxY, (uint32_t)(wndSz.height / _plane.tilePixelHeight + 2 + startRow));
	const uint32_t endCol = min<uint32_t>(maxTileIdxX, (uint32_t)(wndSz.width / _plane.tilePixelWidth + 2 + startCol));

	for (row = startRow; row < endRow; row++)
	{
		rowTileIndex = row % _plane.tilesOnAxisY;
		for (col = startCol; col < endCol; col++)
		{
			colTileIndex = col % _plane.tilesOnAxisX;
			tileId = _plane.tiles[rowTileIndex][colTileIndex];
			if (_plane.tilesImages.count(tileId))
			{
				img = _plane.tilesImages.at(tileId);
				img->position.x = (col + 0.5f) * _plane.tilePixelWidth - parallaxCameraPosX + position.x;
				img->position.y = (row + 0.5f) * _plane.tilePixelHeight - parallaxCameraPosY + position.y;
				img->Draw();
			}
		}
	}
}
