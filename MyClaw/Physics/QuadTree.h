#pragma once

#include "../Objects/BasePlaneObject.h"
#include "../LevelPlane.h"


/// <summary>
/// QuadTree class for efficient spatial partitioning
/// and physics calculations on game surface
/// </summary>
class QuadTree
{
public:
	QuadTree(const DynamicArray<DynamicArray<int32_t>>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions);
	~QuadTree();

	void checkCollides(BaseDynamicPlaneObject* obj) const;

	pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;

	bool isLeaf() const { return !_topLeft && !_topRight && !_bottomLeft && !_bottomRight; }

private:
	QuadTree(Rectangle2D bounds);

	// build the quad-tre from tiles-map.
	// returns true if this tree or one of his children contains rectangles (so it is not empty)
	bool buildTree(const DynamicArray<DynamicArray<int32_t>>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions, int recLevel);
	
	// add rectangle to list if it is not clear and not empty rectangle
	void addRect(const Rectangle2D& rc, uint32_t attr);
	
	// check collides between quad-rects and object.
	// get `objRc` as parameter to save rectangle's calculation in recursion calls
	void checkCollides(BaseDynamicPlaneObject* obj, const Rectangle2D& objRc) const;


	Rectangle2D _bounds;

	QuadTree* _topLeft;
	QuadTree* _topRight;
	QuadTree* _bottomLeft;
	QuadTree* _bottomRight;

	// for leaves we fill the data of map's tiles
	vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }
};
