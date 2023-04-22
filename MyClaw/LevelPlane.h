#pragma once

#include "WwdFile.h"


class LevelPlane : public UIBaseElement
{
public:
	LevelPlane(const WwdPlaneData& planeData);

	void Draw() override;

	bool isMainPlane() const { return _planeData.isMainPlane; }

protected:
	const WwdPlaneData _planeData;
	const uint32_t maxTileIdxX, maxTileIdxY;
};
