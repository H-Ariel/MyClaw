#pragma once

#include "Item.h"


class Warp : public Item
{
public:
	Warp(const WwdObject& obj, Player* player, int8_t type);
	void Logic(uint32_t elapsedTime) override;

private:
	const D2D1_POINT_2F destination;
	const bool oneTimeWarp; // flag to determine whether to delete an object after using it
};
