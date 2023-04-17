#pragma once

#include "BaseEnemy.h"


class Officer : public BaseEnemy
{
public:
	Officer(const WwdObject& obj, Player* player);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
};
