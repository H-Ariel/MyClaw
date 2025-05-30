#pragma once

#include "Projectile.h"


class Stalactite : public Projectile
{
public:
	Stalactite(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	void Reset() override;
	void stopFalling(float collisionSize) override;
	int getDamage() const override;

private:
	Rectangle2D _activityArea;
	const D2D1_POINT_2F _initialPosition;
};
