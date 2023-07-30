#pragma once

#include "Assets-Managers/WwdFile.h"
#include "UIBaseImage.h"


class LevelPlane : public UIBaseElement
{
public:
	LevelPlane();

	virtual void init(); // initialize after loading from WWD

	void Draw() override;

	bool isMainPlane() const { return _isMainPlane; }

	void readPlaneObjects(BufferReader& reader);

protected:
	vector<vector<int32_t>> tiles; // tiles[y][x] = id
	vector<WwdObject> objects;
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
