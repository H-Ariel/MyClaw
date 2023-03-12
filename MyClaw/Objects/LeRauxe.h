#pragma once

#include "BaseEnemy.h"


class LeRauxe : public BaseBoss
{
public:
	LeRauxe(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

private:
	void makeAttack() override;
	bool checkForHurts() override;

	int32_t _attackRest;
	int32_t _hitsCuonter; // count the times CC hit LR
};
