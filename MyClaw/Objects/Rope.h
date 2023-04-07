#pragma once

#include "../BasePlaneObject.h"


class Rope : public BasePlaneObject
{
public:
	Rope(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;

	bool isPassedHalf() const { return _ani->isPassedHalf(); }
};
