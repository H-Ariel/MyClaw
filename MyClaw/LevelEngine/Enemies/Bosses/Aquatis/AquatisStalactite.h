#pragma once

#include "../../../Objects/Projectiles/Projectile.h"


class AquatisStalactite : public Projectile
{
public:
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
	int getDamage() const override;

	void activate();

private:
	AquatisStalactite(const WwdObject& obj);  // create stalactite through Aquatis

	friend class Aquatis;
};
