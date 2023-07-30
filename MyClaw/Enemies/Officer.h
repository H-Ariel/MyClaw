#pragma once

#include "../BaseEnemy.h"


class Officer : public BaseEnemy
{
public:
	Officer(const WwdObject& obj);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
};
