#pragma once

#include "BaseEnemy.h"


class Mercat : public BaseEnemy
{
public:
	Mercat(const WwdObject& obj, Player* player);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int8_t> GetAttackRect() override;

	void makeAttack() override;
};
