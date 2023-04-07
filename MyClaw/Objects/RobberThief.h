#pragma once

#include "BaseEnemy.h"


class RobberThief : public BaseEnemy
{
public:
	RobberThief(const WwdObject& obj, Player* player);

	pair<Rectangle2D, int8_t> GetAttackRect() override;
};
