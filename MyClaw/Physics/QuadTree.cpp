#include "QuadTree.h"
#include "../GlobalObjects.h"
#include "../Objects/Player.h"


#define SAVE_MAX(a, b) if (a < b) a = (b)
#define SAVE_MIN(a, b) if (a > b) a = (b)

// used in `buildTree`
#define MAKE_SUB_TREE(rect, out) \
	tmp = DBG_NEW QuadTree(rect); \
	if (tmp->buildTree(tilesMap, tileDescriptions, recLevel + 1)) out = tmp; \
	else delete tmp;

constexpr float COLLISION_THRESHOLD = 1.0f;
constexpr float LADDER_TOP_THRESHOLD = 32.0f;
constexpr int MAX_TREE_DEPTH = 3;


QuadTree::QuadTree(Rectangle2D bounds)
	: _topLeft(nullptr), _topRight(nullptr), _bottomLeft(nullptr),
	_bottomRight(nullptr), _bounds(bounds)
{
}
QuadTree::QuadTree(const DynamicArray<DynamicArray<int32_t>>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions)
	: _topLeft(nullptr), _topRight(nullptr), _bottomLeft(nullptr), _bottomRight(nullptr),
	_bounds(Rectangle2D(0, 0, (float)tilesMap[0].size() * TILE_SIZE, (float)tilesMap.size() * TILE_SIZE))
{
	buildTree(tilesMap, tileDescriptions, 0);
}
QuadTree::~QuadTree() {
	delete _topLeft;
	delete _topRight;
	delete _bottomLeft;
	delete _bottomRight;
}

void QuadTree::checkCollides(BaseDynamicPlaneObject* obj) const {
	checkCollides(obj, obj->GetRect());
}
void QuadTree::checkCollides(BaseDynamicPlaneObject* obj, const Rectangle2D& objRc) const
{
	if (!isLeaf()) {
		// find correct quad

		if (_topLeft && objRc.intersects(_topLeft->_bounds))
			_topLeft->checkCollides(obj, objRc);
		if (_topRight && objRc.intersects(_topRight->_bounds))
			_topRight->checkCollides(obj, objRc);
		if (_bottomLeft && objRc.intersects(_bottomLeft->_bounds))
			_bottomLeft->checkCollides(obj, objRc);
		if (_bottomRight && objRc.intersects(_bottomRight->_bounds))
			_bottomRight->checkCollides(obj, objRc);

		return;
	}

	// find collisons

	Rectangle2D cumulatedCollision, collisionRc;
	const bool isPlayer = isinstance<Player>(obj);

	auto _addCollision = [&]() { // add `collisionRect` to the `cumulatedCollision`
		// add the collision details to the cummulated collision
		collisionRc.keepSmallest();
		SAVE_MAX(cumulatedCollision.top, collisionRc.top);
		SAVE_MAX(cumulatedCollision.bottom, collisionRc.bottom);
		SAVE_MAX(cumulatedCollision.left, collisionRc.left);
		SAVE_MAX(cumulatedCollision.right, collisionRc.right);
	};
	auto _onGround = [&]() {
		float smallestBottom = collisionRc.getSmallest().bottom;
		// when object is falling or when CC climb down and arrive to the ground
		if ((obj->isFalling() || (isPlayer && GO::player->isClimbing())) &&
			(collisionRc.right > 0 || collisionRc.left > 0) && (0 < smallestBottom && smallestBottom < 16))
		{
			_addCollision();
		}
	};
	auto _onLadder = [&](const Rectangle2D& tileRc) {
		// check if object is at the top of the ladder, so it should stay here (and not fall)
		bool isOnLadderTop = collisionRc.bottom < LADDER_TOP_THRESHOLD;
		if (isPlayer)
			isOnLadderTop = !GO::player->isClimbing() && isOnLadderTop;

		if (isOnLadderTop)
			_onGround(); // ladder top behaves like ground

		if (isPlayer) // let Captain Claw climb
		{
			GO::player->setLadderFlags(isOnLadderTop);
			if (GO::player->isClimbing())
			{
				// set the player position on the ladder easily for the user
				GO::getPlayerPosition().x = (tileRc.left + tileRc.right) / 2;
			}
		}
	};

	for (const auto& [rect, type] : _rects)
	{
		if (objRc.intersects(rect))
		{
			collisionRc = objRc.getCollision(rect);
			switch (type)
			{
			case WwdTileDescription::TileAttribute_Solid: _addCollision(); break;
			case WwdTileDescription::TileAttribute_Ground: _onGround(); break;
			case WwdTileDescription::TileAttribute_Climb: _onLadder(rect); break;
			case WwdTileDescription::TileAttribute_Death: obj->whenTouchDeath(); break;
			}
		}
	}

	if (cumulatedCollision.top > 0) obj->bounceTop();
	if (cumulatedCollision.bottom > 0) obj->stopFalling(cumulatedCollision.bottom);
	if (cumulatedCollision.left > 0) obj->stopMovingLeft(cumulatedCollision.left);
	if (cumulatedCollision.right > 0) obj->stopMovingRight(cumulatedCollision.right);
}

pair<float, float> QuadTree::getEnemyRange(D2D1_POINT_2F enemyPos, const float minX, const float maxX) const
{
	if (!isLeaf()) {
		// calculate range from all quads and combine the result

		Rectangle2D objRc(
			enemyPos.x - TILE_SIZE, enemyPos.y - TILE_SIZE,
			enemyPos.x + TILE_SIZE, enemyPos.y + TILE_SIZE
		);

		vector<pair<float, float>> pairs;

		if (_topLeft && objRc.intersects(_topLeft->_bounds))
			pairs.push_back(_topLeft->getEnemyRange(enemyPos, minX, maxX));
		if (_topRight && objRc.intersects(_topRight->_bounds))
			pairs.push_back(_topRight->getEnemyRange(enemyPos, minX, maxX));
		if (_bottomLeft && objRc.intersects(_bottomLeft->_bounds))
			pairs.push_back(_bottomLeft->getEnemyRange(enemyPos, minX, maxX));
		if (_bottomRight && objRc.intersects(_bottomRight->_bounds))
			pairs.push_back(_bottomRight->getEnemyRange(enemyPos, minX, maxX));

		auto it = pairs.begin();
		if (it == pairs.end())
			return {};
		float nleft = it->first, nright = it->second;
		for (; it != pairs.end(); ++it) {
			SAVE_MAX(nleft, it->first);
			SAVE_MIN(nright, it->second);
		}

		return { nleft, nright };
	}
	else {
		Rectangle2D enemyRect(enemyPos.x, enemyPos.y, enemyPos.x, enemyPos.y + 80);

		for (const auto& [rect, type] : _rects)
		{
			if ((type == WwdTileDescription::TileAttribute_Solid || type == WwdTileDescription::TileAttribute_Ground)
				&& rect.intersects(enemyRect)) // if enemy walk on solid/ground he has the same bounds
			{
				float left = rect.left + 32, right = rect.right - 32;

				if (minX != 0 && maxX != 0)
				{
					SAVE_MAX(left, minX);
					SAVE_MIN(right, maxX);
				}

				return { left, right };
			}
		}

		return { enemyPos.x - 32, enemyPos.x + 32 };
	}
}

bool QuadTree::buildTree(const DynamicArray<DynamicArray<int32_t>>& tilesMap, const DynamicArray<WwdTileDescription>& tileDescriptions, int recLevel) {
	if (recLevel < MAX_TREE_DEPTH) {
		// find all quads. if quad has no sub-rectangles in it - delete this quads.

		float centerX = (_bounds.left + _bounds.right) / 2;
		float centerY = (_bounds.top + _bounds.bottom) / 2;
		QuadTree* tmp;

		MAKE_SUB_TREE(Rectangle2D(_bounds.left, _bounds.top, centerX, centerY), _topLeft);
		MAKE_SUB_TREE(Rectangle2D(centerX, _bounds.top, _bounds.right, centerY), _topRight);
		MAKE_SUB_TREE(Rectangle2D(_bounds.left, centerY, centerX, _bounds.bottom), _bottomLeft);
		MAKE_SUB_TREE(Rectangle2D(centerX, centerY, _bounds.right, _bounds.bottom), _bottomRight);

		return _topLeft || _topRight || _bottomLeft || _bottomRight;
	}
	else {
		// fill quad with match rectangles:
		// map of all rectangles that BaseDynamicPlaneObjects can collide with 

		const uint32_t startX = (uint32_t)(_bounds.left / TILE_SIZE);
		const uint32_t endX = (uint32_t)(_bounds.right / TILE_SIZE);
		const uint32_t startY = (uint32_t)(_bounds.top / TILE_SIZE);
		const uint32_t endY = (uint32_t)(_bounds.bottom / TILE_SIZE);

		WwdTileDescription tileDesc;
		Rectangle2D tileRc;
		uint32_t i, j;
		float x1, x2, y1, y2;

		for (i = startY; i < endY; i++) {
			for (j = startX; j < endX; j++) {
				if (tilesMap[i][j] == EMPTY_TILE) tileDesc = {};
				else tileDesc = tileDescriptions[tilesMap[i][j]];

				tileRc.left = (float)(j * TILE_SIZE);
				tileRc.top = (float)(i * TILE_SIZE);
				tileRc.right = tileRc.left + TILE_SIZE - 1;
				tileRc.bottom = tileRc.top + TILE_SIZE - 1;

				switch (tileDesc.type)
				{
				case WwdTileDescription::TileType_Single:
					addRect(tileRc, tileDesc.insideAttrib);
					break;

				case WwdTileDescription::TileType_Double:
					/*
					create 9 rectangles from `tileDesc.rect` and `tileRc`:
					all rectangles create `tileRc` except the middle one which creates `tileDesc.rect`
					o | o | o
					--+---+--
					o | i | o
					--+---+--
					o | o | o
					(o - outside, i - inside)
					*/
					x1 = tileRc.left + tileDesc.rect.left;
					x2 = tileRc.left + tileDesc.rect.right;
					y1 = tileRc.top + tileDesc.rect.top;
					y2 = tileRc.top + tileDesc.rect.bottom;
					addRect({ tileRc.left, tileRc.top, x1, y1 }, tileDesc.outsideAttrib);
					addRect({ x1, tileRc.top, x2, y1 }, tileDesc.outsideAttrib);
					addRect({ x2, tileRc.top, tileRc.right, y1 }, tileDesc.outsideAttrib);
					addRect({ tileRc.left, y1, x1, y2 }, tileDesc.outsideAttrib);
					addRect({ x1, y1, x2, y2 }, tileDesc.insideAttrib);
					addRect({ x2, y1, tileRc.right, y2 }, tileDesc.outsideAttrib);
					addRect({ tileRc.left, y2, x1, tileRc.bottom }, tileDesc.outsideAttrib);
					addRect({ x1, y2, x2, tileRc.bottom }, tileDesc.outsideAttrib);
					addRect({ x2, y2, tileRc.right, tileRc.bottom }, tileDesc.outsideAttrib);
					break;
				}
			}
		}

		// TODO !!! find better merge algorithm

		// this loop combines rectangles that are next to each other (according to x axis)
		for (i = 0; i < _rects.size(); i++) {
			for (j = i + 1; j < _rects.size(); j++) {
				if (_rects[i].second == _rects[j].second &&
					abs(_rects[i].first.top - _rects[j].first.top) <= COLLISION_THRESHOLD &&
					abs(_rects[i].first.bottom - _rects[j].first.bottom) <= COLLISION_THRESHOLD &&
					abs(_rects[i].first.right - _rects[j].first.left) <= COLLISION_THRESHOLD)
				{
					_rects[i].first = Rectangle2D(_rects[i].first.left, _rects[i].first.top, _rects[j].first.right, _rects[i].first.bottom);
					_rects.erase(_rects.begin() + j);
					j--;
				}
			}
		}

		// this loop combines rectangles that are next to each other (according to y axis)
		for (i = 0; i < _rects.size(); i++) {
			for (j = i + 1; j < _rects.size(); j++) {
				if (_rects[i].second == _rects[j].second &&
					abs(_rects[i].first.left - _rects[j].first.left) <= COLLISION_THRESHOLD &&
					abs(_rects[i].first.right - _rects[j].first.right) <= COLLISION_THRESHOLD &&
					abs(_rects[i].first.bottom - _rects[j].first.top) <= COLLISION_THRESHOLD)
				{
					_rects[i].first = Rectangle2D(_rects[i].first.left, _rects[i].first.top, _rects[i].first.right, _rects[j].first.bottom);
					_rects.erase(_rects.begin() + j);
					j--;
				}
			}
		}
	}

	return !_rects.empty();
}

void QuadTree::addRect(const Rectangle2D& rc, uint32_t attr) {
	if (attr != WwdTileDescription::TileAttribute_Clear && rc.left != rc.right && rc.top != rc.bottom)
	{
		_rects.push_back({ rc, attr });
	}
}

/*
void QuadTree::print(string t) { // TODO delete this func
	printf("%s{ %.2f, %.2f, %.2f, %.2f }\n", t.c_str(), _bounds.left, _bounds.top, _bounds.right, _bounds.bottom);

	if (isLeaf()) {
		printf("%s%s%d rects\n", t.c_str(), t.c_str(), (int)_rects.size());
	}
	else {
		if (_topLeft) _topLeft->print(t + "   ");
		if (_topRight) _topRight->print(t + "   ");
		if (_bottomLeft) _bottomLeft->print(t + "   ");
		if (_bottomRight) _bottomRight->print(t + "   ");
	}
}
*/
