#pragma once

#include "BaseEnemy.h"


class RobberThief : public BaseEnemy
{
public:
	RobberThief(const WwdObject& obj);

	pair<Rectangle2D, int> GetAttackRect() override;
};
