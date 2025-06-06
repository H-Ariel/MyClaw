#pragma once

#include "EnemyProjectile.h"


class CrabBomb : public EnemyProjectile
{
public:
	CrabBomb(const WwdObject& obj);
	~CrabBomb();
};
