#pragma once

#include "BaseEnemy.h"


class BearSailor : public BaseEnemy
{
public:
	BearSailor(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

private:
	void makeAttack() override;
};
