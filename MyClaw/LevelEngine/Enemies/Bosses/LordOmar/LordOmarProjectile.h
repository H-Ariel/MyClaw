#pragma once

#include "../../../Objects/Projectiles/EnemyProjectile.h"


class LordOmarProjectile : public EnemyProjectile
{
public:
	LordOmarProjectile(D2D1_POINT_2F pos, float speedX);
	Rectangle2D GetRect() override;
};
