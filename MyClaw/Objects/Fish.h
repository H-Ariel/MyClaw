#pragma once

#include "BaseEnemy.h"

class Fish : public BaseEnemy
{
public:
	Fish(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int8_t> GetAttackRect() override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void makeAttack() override;

private:
	const float _yPos;
};
