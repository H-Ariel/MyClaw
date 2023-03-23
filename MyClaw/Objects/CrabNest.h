#pragma once

#include "HermitCrab.h"


class CrabNest : public BaseStaticPlaneObject
{
public:
	CrabNest(const WwdObject& obj, Player* player);
	~CrabNest();

	void Logic(uint32_t elapsedTime) override;

private:
	vector<HermitCrab*> _crabs;
};
