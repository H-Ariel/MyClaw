#pragma once

#include "BaseEnemy.h"

class Siren : public BaseEnemy
{
public:
	Siren(const WwdObject& obj, Player* player);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int8_t> GetAttackRect() override;

	void makeAttack() override;
};
