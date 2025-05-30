#pragma once

#include "../../../Objects/EnemyProjectile.h"


class GabrielBomb : public EnemyProjectile
{
public:
	GabrielBomb(const WwdObject& obj);
	~GabrielBomb();
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
};
