#pragma once

#include "BaseEnemy.h"


class Siren : public BaseEnemy
{
public:
	Siren(const WwdObject& obj);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void makeAttack(float deltaX, float deltaY) override;
};
