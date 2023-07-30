#pragma once

#include "../BaseEnemy.h"


class PegLeg : public BaseEnemy
{
public:
	PegLeg(const WwdObject& obj);
	
	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
};
