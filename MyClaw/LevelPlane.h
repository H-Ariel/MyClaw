#pragma once

#include "RezParser/WwdFile.h"
#include "BasePlaneObject.h"


class LevelPlane : public UIBaseElement
{
public:
	LevelPlane(WapWwd* wwd, WwdPlane* wwdPlane);

	void Draw() override;
	virtual void init();
	virtual void updateObject(WwdObject& obj); // called from `init`
	virtual void addObject(const WwdObject& obj); // called from `init`

	bool isVisible;

protected:
	vector<BasePlaneObject*> _objects;
	map<int, shared_ptr<UIBaseImage>> tilesImages; // [id]=image
	WapWwd* _wwd;
	WwdPlane* _wwdPlane;
	int maxTileIdxX, maxTileIdxY;


	friend class WapWorld;
	friend class PhysicsManager;
};
