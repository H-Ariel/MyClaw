#pragma once

#include "BaseEnemy.h"


class HermitCrab : public BaseEnemy
{
public:
	HermitCrab(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

	bool isDuck() const override;
	bool isTakeDamage() const override;

private:
	void makeAttack() override;

	int32_t _attackRest; // rest time between attack
};
