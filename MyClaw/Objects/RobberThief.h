#pragma once

#include "BaseEnemy.h"


class RobberThief : public BaseEnemy
{
public:
	RobberThief(const WwdObject& obj, Player* player);

	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;
};
