#pragma once

#include "BaseEnemy.h"


class BearSailor : public BaseEnemy
{
public:
	BearSailor(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

private:
	void makeAttack() override;
};
