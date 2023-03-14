#pragma once

#include "BaseEnemy.h"


class TownGuard : public BaseEnemy
{
public:
	TownGuard(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

private:
	enum class Type : int8_t { Guard1, Guard2 }; // 1-white, 2-brown
	
	void makeAttack() override;

	const Type _type;
};
