#pragma once

#include "BaseEnemy.h"


class RedTailPirate : public BaseEnemy
{
public:
	RedTailPirate(const WwdObject& obj, Player* player);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int8_t> GetAttackRect() override;
};
