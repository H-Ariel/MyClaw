#pragma once

#include "UIBaseElement.h"
#include "WwdFile.h"


class LevelPlane : public UIBaseElement
{
public:
	LevelPlane(const WwdPlane& plane);

	void Draw() override;

protected:
	const WwdPlane& _plane;
	const uint32_t maxTileIdxX, maxTileIdxY;
};
