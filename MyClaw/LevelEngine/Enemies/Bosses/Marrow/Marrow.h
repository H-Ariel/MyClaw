#pragma once

#include "../BaseBoss.h"

class MarrowState;


class Marrow : public BaseBoss
{
public:
	Marrow(const WwdObject& obj);
	~Marrow();

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;

	bool isInLefttSide() const { return !_side; }
	bool isInRightSide() const { return _side; }

	MarrowState* getState() const { return _state; }
	void switchState(MarrowState* newState);

	static Marrow* getInstance() { return _MarrowInstance; }


private:
	void makeAttack(float deltaX, float deltaY) override;
	bool checkForHurts() override;
	bool _side; // false- Left, true- Right
	MarrowState* _state;

	static Marrow* _MarrowInstance;

	friend class ClawAttackMarrowState;
	friend class ParrotTakeMarrowState;
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

	static constexpr float PARROT_SPEED = 0.4f;


private:
	bool checkForHurts() override;

	const Rectangle2D _flyRect;
	const D2D1_POINT_2F _initialPosition;
	int _hitsCounter;

	friend class ParrotAttackClawState;
	friend class ClawAttackMarrowState;
	friend class ParrotTakeMarrowState;
};


class MarrowFloor : public BaseStaticPlaneObject
{
public:
	MarrowFloor(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;
	

private:
	void moveFloor(uint32_t elapsedTime, bool updated);

	const float _minX, _maxX, _speedX;

	static int8_t floorCounter; // use to sync floors

	friend class FirstFloorsRemoveState;
	friend class AddFloorState;
	friend class ParrotTakeMarrowState;
};
