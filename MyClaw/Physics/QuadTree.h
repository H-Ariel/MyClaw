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
	QuadTree(const Dynamic2DArray<int32_t>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions);
	~QuadTree();

	void checkCollides(BaseDynamicPlaneObject* obj) const;

	pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;


private:
	class QuadTreeNode {
	public:
		QuadTreeNode(Rectangle2D bounds);
		~QuadTreeNode();

		bool isLeaf() const { return !_topLeft && !_topRight && !_bottomLeft && !_bottomRight; }

		// check collides between quad-rects and object.
		// get `objRc` as parameter to save rectangle's calculation in recursion calls
		void checkCollides(BaseDynamicPlaneObject* obj, const Rectangle2D& objRc) const;

		// returns the range [minX, maxX] that enemy can walk on plane according to map
		pair<float, float> getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const;

		// build the quad-tre from tiles-map.
		// returns true if this tree or one of his children contains rectangles (so it is not empty)
		bool buildTree(const Dynamic2DArray<int32_t>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions, int recLevel);

	private:
		// add rectangle to list if it is not clear and not empty rectangle
		void addRect(const Rectangle2D& rc, uint32_t attr);


		Rectangle2D _bounds;

		QuadTreeNode* _topLeft;
		QuadTreeNode* _topRight;
		QuadTreeNode* _bottomLeft;
		QuadTreeNode* _bottomRight;

		// for leaves we fill the data of map's tiles
		vector<pair<Rectangle2D, uint32_t>> _rects; // { rc, WwdTileDescription::WwdTileAttributeFlags }

		// save the tiles map. used mainly to check if ladder is finish at quad bound.
		const Dynamic2DArray<int32_t>* p_tilesMap;
		const DynamicArray<WwdTileDescription>* p_tileDescriptions;
	};
	
	
	QuadTreeNode* _root;
};
