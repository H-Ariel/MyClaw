#pragma once

#include "BaseEnemy.h"


class CutThroat : public BaseEnemy
{
public:
	CutThroat(const WwdObject& obj, Player* player);

	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;
};
