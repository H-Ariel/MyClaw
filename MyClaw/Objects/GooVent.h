#pragma once

#include "../BasePlaneObject.h"


class GooVent : public BaseStaticPlaneObject
{
public:
	GooVent(const WwdObject& obj, Player* player);

	int getDamage() const;

private:
	const int _damage;
};

