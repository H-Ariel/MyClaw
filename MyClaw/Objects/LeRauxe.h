#pragma once

#include "BaseEnemy.h"


class LeRauxe : public BaseBoss
{
public:
	LeRauxe(const WwdObject& obj);

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
};
