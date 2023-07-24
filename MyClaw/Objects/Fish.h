#pragma once

#include "BaseEnemy.h"


class Fish : public BaseEnemy
{
public:
	Fish(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void makeAttack() override;

private:
	const float _yPos;
};
