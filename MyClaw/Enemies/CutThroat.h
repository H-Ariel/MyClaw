#pragma once

#include "../BaseEnemy.h"


class CutThroat : public BaseEnemy
{
public:
	CutThroat(const WwdObject& obj);

	pair<Rectangle2D, int> GetAttackRect() override;
};
