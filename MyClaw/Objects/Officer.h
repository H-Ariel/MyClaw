#pragma once

#include "BaseEnemy.h"


class Officer : public BaseEnemy
{
public:
	Officer(const WwdObject& obj, Player* player);

	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;
};
