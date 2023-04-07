#pragma once

#include "BaseEnemy.h"


class CrazyHook : public BaseEnemy
{
public:
	CrazyHook(const WwdObject& obj, Player* player);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int8_t> GetAttackRect() override;
};
