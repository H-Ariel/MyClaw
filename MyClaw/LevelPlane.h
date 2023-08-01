#pragma once

#include "Assets-Managers/WwdFile.h"
#include "BasePlaneObject.h"


class LevelPlane : public UIBaseElement
{
public:
	LevelPlane();

	void Draw() override;
	virtual void readPlaneObjects(BufferReader& reader);
	virtual void addObject(const WwdObject& obj); // called from `readPlaneObjects`
	bool isMainPlane() const { return _isMainPlane; }

protected:
	vector<vector<int32_t>> tiles; // tiles[y][x] = id
	vector<BasePlaneObject*> _objects;
	map<int32_t, shared_ptr<UIBaseImage>> tilesImages; // [id]=image
	ColorF fillColor;
	uint32_t tilesOnAxisX, tilesOnAxisY;
	uint32_t maxTileIdxX, maxTileIdxY;
	int32_t ZCoord;
	float movementPercentX, movementPercentY;
	bool _isMainPlane;


	friend class WapWorld;
	friend class PhysicsManager;
};
