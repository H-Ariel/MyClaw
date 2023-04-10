#pragma once

#include "BaseEnemy.h"


class Marrow : public BaseBoss
{
public:
	Marrow(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	pair<Rectangle2D, int8_t> GetAttackRect() override;

private:
	bool checkForHurts() override;

};

class MarrowParrot : public BaseEnemy
{
public:
	MarrowParrot(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	
	pair<Rectangle2D, int8_t> GetAttackRect() override;

	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void bounceTop() override;

private:
	bool checkForHurts() override;

	enum class State : int8_t {
		Fly,
		TakeMarrow
	};

	const Rectangle2D _flyRect;
	int8_t _hitsCounter;
	State _state;
};



class MarrowFloor : public BaseStaticPlaneObject
{
public:
	MarrowFloor(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

private:
	const float _minX, _maxX, _speedX;
};
