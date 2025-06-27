#pragma once

#include "BaseBoss.h"


class Wolvington : public BaseBoss
{
public:
	Wolvington(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;

private:
	bool PreLogic(uint32_t elapsedTime) override;
	void makeAttack(float deltaX, float deltaY) override;
	bool checkForHurts() override;

	int _magicAttackCuonter; // count the times W attack CC with magic
};
