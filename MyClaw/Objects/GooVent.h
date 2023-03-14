#pragma once

#include "../BasePlaneObject.h"


class GooVent : public BaseStaticPlaneObject
{
public:
	GooVent(const WwdObject& obj, Player* player);

	int8_t getDamage() const;

private:
	const int32_t _damage;
};

