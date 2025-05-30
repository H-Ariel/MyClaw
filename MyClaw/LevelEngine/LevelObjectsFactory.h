#pragma once

#include "Objects/BasePlaneObject.h"


class LevelObjectsFactory {
public:
	LevelObjectsFactory(WapWwd* wwd) : _wwd(wwd) {}

	BasePlaneObject* createObject(const WwdObject& obj);

private:
	WapWwd* _wwd;
};
