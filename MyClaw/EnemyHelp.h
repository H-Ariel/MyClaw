#pragma once

/*
 * Help classes for enemies
 */

#include "Objects/Item.h"


// This class is responsible for leaving the enemy from the screen
class EnemyFallDeath : public BaseDynamicPlaneObject
{
public:
	EnemyFallDeath(const WwdObject& obj, shared_ptr<UIAnimation> deadAni);
	void Logic(uint32_t elapsedTime) override;
};

// This class is responsible for the gem that the boss leaves
class BossGem : public Item
{
public:
	BossGem(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	D2D1_POINT_2F _destination;
};
