#pragma once

#include "BaseEnemy.h"


class PegLeg : public BaseEnemy
{
public:
	PegLeg(const WwdObject& obj, Player* player);
	
	Rectangle2D GetRect() override;
	pair<Rectangle2D, int8_t> GetAttackRect() override;
};
