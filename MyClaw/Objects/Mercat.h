#pragma once

#include "BaseEnemy.h"


class Mercat : public BaseEnemy
{
public:
	Mercat(const WwdObject& obj, Player* player);

	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

	void makeAttack() override;
};
