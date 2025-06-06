#pragma once

#include "BaseEnemy.h"


class Rat : public BaseEnemy
{
public:
	Rat(const WwdObject& obj);

	void makeAttack(float deltaX, float deltaY) override;
	pair<Rectangle2D, int> GetAttackRect() override;

	bool isDuck() const override;
	bool isTakeDamage() const override;
};
