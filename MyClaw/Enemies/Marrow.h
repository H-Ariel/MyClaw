#pragma once

#include "../BaseEnemy.h"


class Marrow : public BaseBoss
{
public:
	Marrow(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

	pair<Rectangle2D, int> GetAttackRect() override;

	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;

private:
	void makeAttack() override;
	bool checkForHurts() override;

};

class MarrowParrot : public BaseEnemy
{
public:
	MarrowParrot(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	
	pair<Rectangle2D, int> GetAttackRect() override;

	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void bounceTop() override;

private:
	bool checkForHurts() override;

	const Rectangle2D _flyRect;
	const D2D1_POINT_2F _initialPosition;
	int _hitsCounter;
};



class MarrowFloor : public BaseStaticPlaneObject
{
public:
	MarrowFloor(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;

private:
	const float _minX, _maxX, _speedX;
};
