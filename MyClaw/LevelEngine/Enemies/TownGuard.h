#pragma once

#include "BaseEnemy.h"


class TownGuard : public BaseEnemy
{
public:
	TownGuard(const WwdObject& obj);

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

private:
	enum class Type : int8_t { Guard1, Guard2 }; // 1-white, 2-brown
	const Type _type;
};
