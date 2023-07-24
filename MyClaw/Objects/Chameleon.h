#pragma once

#include "BaseEnemy.h"


class Chameleon : public BaseEnemy
{
public:
	Chameleon(const WwdObject& obj);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void makeAttack() override;
};
