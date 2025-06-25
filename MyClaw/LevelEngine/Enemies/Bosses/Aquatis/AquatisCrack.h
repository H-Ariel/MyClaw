#pragma once

#include "../../../Objects/Projectiles/Projectile.h"


class AquatisCrack : public BaseStaticPlaneObject
{
public:
	AquatisCrack(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

private:
	Projectile* _lastDynamite;
};
