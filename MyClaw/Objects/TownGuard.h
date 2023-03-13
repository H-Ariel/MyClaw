#pragma once

#include "BaseEnemy.h"


class TownGuard : public BaseEnemy
{
public:
	TownGuard(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

private:
	enum class Type : int8_t { Guard1, Guard2 };
	
	void makeAttack() override;

	int32_t _attackRest;
	const Type _type;
};
