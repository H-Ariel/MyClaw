#pragma once

#include "Objects/BasePlaneObject.h"
#include "LevelPlane.h"


class QuadTree
{
public:
	QuadTree(const DynamicArray<DynamicArray<int32_t>>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions);
	~QuadTree();

	void checkCollides(BaseDynamicPlaneObject* obj) const;

	pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;

	bool isLeaf() const { return !_topLeft && !_topRight && !_bottomLeft && !_bottomRight; }
	bool hasRects() const;

private:
	QuadTree(const DynamicArray<DynamicArray<int32_t>>& tilesMap,
		const DynamicArray<WwdTileDescription>& tileDescriptions, Rectangle2D bounds, int recLevel);

	void buildTree(const DynamicArray<DynamicArray<int32_t>>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions, int recLevel);
	void addRect(const Rectangle2D& rc, uint32_t attr);
	void checkCollides(BaseDynamicPlaneObject* obj, const Rectangle2D& objRc) const; // get `objRc` as parameter to save rectangle's calculation in recursion calls


	Rectangle2D _bounds;

	QuadTree* _topLeft;
	QuadTree* _topRight;
	QuadTree* _bottomLeft;
	QuadTree* _bottomRight;

	// for leaves we fill the data of map's tiles
	vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }
};
