#pragma once

#include "BasePlaneObject.h"


class Rope : public BasePlaneObject
{
public:
	Rope(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;

	bool isPassedHalf() const { return _ani->isPassedHalf(); }

private:
	D2D1_POINT_2F _edgePos;
};
