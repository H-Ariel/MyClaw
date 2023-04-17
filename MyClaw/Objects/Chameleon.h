#pragma once

#include "BaseEnemy.h"


class Chameleon : public BaseEnemy
{
public:
	Chameleon(const WwdObject& obj, Player* player);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void makeAttack() override;
};
