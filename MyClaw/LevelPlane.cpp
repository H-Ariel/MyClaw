#include "LevelPlane.h"
#include "WindowManager.h"


LevelPlane::LevelPlane(const WwdPlaneData& planeData)
	: _planeData(planeData),
	maxTileIdxX(planeData.isWrappedX ? INT32_MAX : planeData.tilesOnAxisX),
	maxTileIdxY(planeData.isWrappedY ? INT32_MAX : planeData.tilesOnAxisY)
{
}

void LevelPlane::Draw()
{
	shared_ptr<UIBaseImage> img;
	int row, col, tileId, rowTileIndex;

	const D2D1_SIZE_F wndSz = WindowManager::getSize();
	const float parallaxCameraPosX = position.x * _planeData.movementPercentX;
	const float parallaxCameraPosY = position.y * _planeData.movementPercentY;
	const int startRow = int(parallaxCameraPosY / TILE_SIZE);
	const int startCol = int(parallaxCameraPosX / TILE_SIZE);
	const int endRow = min<int>(maxTileIdxY, int(wndSz.height / TILE_SIZE + 2 + startRow));
	const int endCol = min<int>(maxTileIdxX, int(wndSz.width / TILE_SIZE + 2 + startCol));

	for (row = startRow; row < endRow; row++)
	{
		rowTileIndex = row % _planeData.tilesOnAxisY;
		for (col = startCol; col < endCol; col++)
		{
			tileId = _planeData.tiles[rowTileIndex][col % _planeData.tilesOnAxisX];
			if (_planeData.tilesImages.count(tileId))
			{
				img = _planeData.tilesImages.at(tileId);
				img->position.x = (col + 0.5f) * TILE_SIZE - parallaxCameraPosX + position.x;
				img->position.y = (row + 0.5f) * TILE_SIZE - parallaxCameraPosY + position.y;
				img->Draw();
			}
		}
	}
}
