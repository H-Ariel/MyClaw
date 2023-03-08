#pragma once

#include "../BasePlaneObject.h"


class Rope : public BasePlaneObject
{
public:
	Rope(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	D2D1_RECT_F GetRect() override;

	bool isPassedHalf() const;

private:
	D2D1_POINT_2F getRopeEdgePos();
};
