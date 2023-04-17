#pragma once

#include "BaseEnemy.h"


class HermitCrab : public BaseEnemy
{
public:
	HermitCrab(const WwdObject& obj, Player* player, bool isFromNest = false);

	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
	pair<Rectangle2D, int> GetAttackRect() override;
	bool isDuck() const override;
	bool isTakeDamage() const override;

private:
	void makeAttack() override;

	bool _isFromNest;
};
