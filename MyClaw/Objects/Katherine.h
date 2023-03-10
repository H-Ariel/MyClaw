#pragma once

#include "BaseEnemy.h"


class Katherine : public BaseBoss
{
public:
	Katherine(const WwdObject& obj, Player* player);

	// TODO: write code
//	void Logic(uint32_t elapsedTime) override;

	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;
};
