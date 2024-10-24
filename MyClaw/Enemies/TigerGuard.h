#pragma once

#include "BaseEnemy.h"


class TigerGuard : public BaseEnemy
{
public:
	TigerGuard(const WwdObject& obj);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
};
