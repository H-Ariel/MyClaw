#pragma once

#include "Projectile.h"


class LavaMouth : public BaseDamageObject //Projectile
{
public:
	LavaMouth(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	bool isDamage() const override;

private:
	int32_t _startTimeDelay; // in milliseconds
};
