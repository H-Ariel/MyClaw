#pragma once

#include "EnemyProjectile.h"


class SirenScream: public EnemyProjectile
{
public:
	SirenScream(const WwdObject& obj, int delay);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
};
