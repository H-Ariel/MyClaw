#pragma once

#include "../BasePlaneObject.h"


class GooVent : public BaseDamageObject
{
public:
	GooVent(const WwdObject& obj);
	bool isDamage() const override;
};
