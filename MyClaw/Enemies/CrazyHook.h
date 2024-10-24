#pragma once

#include "BaseEnemy.h"


class CrazyHook : public BaseEnemy
{
public:
	CrazyHook(const WwdObject& obj);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
};
