#pragma once

#include "../BaseEnemy.h"


class Chameleon : public BaseEnemy
{
public:
	Chameleon(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void makeAttack(float deltaX, float deltaY) override;
	bool checkForHurts() override;

private:
	float _opacity;
};
