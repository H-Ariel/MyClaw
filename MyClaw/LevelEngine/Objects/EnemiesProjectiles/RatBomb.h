#pragma once

#include "EnemyProjectile.h"


class RatBomb : public EnemyProjectile
{
public:
	RatBomb(const WwdObject& obj);
	~RatBomb();
};
