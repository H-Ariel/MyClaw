#pragma once

#include "../BaseEnemy.h"


class RedTailPirate : public BaseEnemy
{
public:
	RedTailPirate(const WwdObject& obj, bool noTreasures = false);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
};