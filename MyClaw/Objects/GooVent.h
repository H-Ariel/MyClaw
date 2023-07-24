#pragma once

#include "../BasePlaneObject.h"


class GooVent : public BaseDamageObject
{
public:
	GooVent(const WwdObject& obj, Player* player);
	bool isDamage() const override;
};
