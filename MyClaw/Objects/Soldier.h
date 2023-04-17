#pragma once

#include "BaseEnemy.h"


class Soldier : public BaseEnemy
{
public:
	Soldier(const WwdObject& obj, Player* player);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
};
