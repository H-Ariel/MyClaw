#pragma once

#include "BasePlaneObject.h"


class LavaMouth : public BaseDamageObject
{
public:
	LavaMouth(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	bool isDamage() const override;
};
