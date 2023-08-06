#pragma once

#include "Projectile.h"


class Stalactite : public Projectile
{
public:
	Stalactite(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
	int getDamage() const override;

private:
	Rectangle2D _activityArea;
};
