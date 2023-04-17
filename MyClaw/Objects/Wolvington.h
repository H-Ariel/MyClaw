#pragma once

#include "BaseEnemy.h"


class Wolvington : public BaseBoss
{
public:
	Wolvington(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;

private:
	bool PreLogic(uint32_t elapsedTime) override;
	void makeAttack() override;
	bool checkForHurts() override;

	int32_t _magicAttackCuonter; // count the times W attack CC with magic
};
