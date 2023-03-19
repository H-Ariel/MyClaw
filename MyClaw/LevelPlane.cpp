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
	int32_t row, col;

	const D2D1_SIZE_F wndSz = WindowManager::getSize();
	const float parallaxCameraPosX = position.x * _plane.movementPercentX;
	const float parallaxCameraPosY = position.y * _plane.movementPercentY;
	const int32_t startCol = (int32_t)(parallaxCameraPosX / _plane.tilePixelWidth);
	const int32_t startRow = (int32_t)(parallaxCameraPosY / _plane.tilePixelHeight);
	const int32_t endRow = min<int32_t>(maxTileIdxY, (int32_t)(wndSz.height / _plane.tilePixelHeight + 2 + startRow));
	const int32_t endCol = min<int32_t>(maxTileIdxX, (int32_t)(wndSz.width / _plane.tilePixelWidth + 2 + startCol));

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
