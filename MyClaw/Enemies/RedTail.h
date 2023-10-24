#pragma once

#include "../BaseEnemy.h"


class RedTail : public BaseBoss
{
public:
	RedTail(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;


private:
	void makeAttack(float deltaX, float deltaY) override;
	bool checkForHurts() override;
};


class RedTailSpikes : public BaseDamageObject
{
public:
	RedTailSpikes(const WwdObject& obj);
	bool isDamage() const override;
};


class RedTailWind : public BasePlaneObject
{
public:
	RedTailWind(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void activate();

private:
	int _windTimeCounter; // time left to wind
};
