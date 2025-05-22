#pragma once

#include "BaseEnemy.h"

class Marrow;
class MarrowParrot;
class MarrowFloor;

class MarrowState {
public:
	enum class StateType : int8_t {
		FirstFloorsRemove,
		ParrotAttackClaw,
		AddFloor,
		ClawAttackMarrow,
		ParrotTakeMarrow
	};

	virtual ~MarrowState() = default;
	virtual void MarrowLogic(Marrow* marrow, uint32_t elapsedTime) = 0;
	virtual void ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) = 0;
	virtual void FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) = 0;

	// TODO: save pointers to objects instead of func arguments?

	StateType getType() const { return _type; }

protected:
	MarrowState(StateType type) : _type(type) {}

	const StateType _type;
};


class Marrow : public BaseBoss
{
public:
	Marrow(const WwdObject& obj);
	~Marrow();

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;

	enum class Side : int8_t { Right = 0, Left = 1 };
	Side getSide() const { return _side; }

	void ClawAttackMarrowStateLogic(uint32_t elapsedTime);
	void ParrotTakeMarrowStateLogic(uint32_t elapsedTime);

	MarrowState* getState() const { return _state; }
	void switchState(MarrowState* newState);

private:
	void makeAttack(float deltaX, float deltaY) override;
	bool checkForHurts() override;
	Side _side;
	MarrowState* _state;
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

	void ParrotAttackClawStateLogic(uint32_t elapsedTime);
	void ClawAttackMarrowStateLogic(uint32_t elapsedTime);
	void ParrotTakeMarrowStateLogic(uint32_t elapsedTime);


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
	
	void AddFloorStateLogic(uint32_t elapsedTime);
	void ParrotTakeMarrowStateLogic(uint32_t elapsedTime);
	void FirstFloorsRemoveStateLogic(uint32_t elapsedTime);

private:
	void removeFloor(uint32_t elapsedTime);

	const float _minX, _maxX, _speedX;

	static int8_t floorCounter; // use to sync floors
};
