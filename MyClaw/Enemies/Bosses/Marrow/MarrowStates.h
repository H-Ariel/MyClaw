#pragma once

#include "Marrow.h"


/*
States of the fight with Marrow:
1. CC enter to boss area, floors will open
2. Parrot attack CC [after 3 hits see next state]
3. floor closed
4. CC go to Marrow and attack him [after 5 hits see next state]
5. Floors open and parrot takes Marrow to the other side [back to state 2]
*/
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


class FirstFloorsRemoveState : public MarrowState {
public:
	FirstFloorsRemoveState() : MarrowState(StateType::FirstFloorsRemove) {}

	void MarrowLogic(Marrow* marrow, uint32_t elapsedTime) override;
	void ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) override;
	void FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) override;
};

class ParrotAttackClawState : public MarrowState {
public:
	ParrotAttackClawState() : MarrowState(StateType::ParrotAttackClaw) {}

	void MarrowLogic(Marrow* marrow, uint32_t elapsedTime) override;
	void ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) override;
	void FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) override;
};

class AddFloorState : public MarrowState {
public:
	AddFloorState() : MarrowState(StateType::AddFloor) {}

	void MarrowLogic(Marrow* marrow, uint32_t elapsedTime) override;
	void ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) override;
	void FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) override;
};

class ClawAttackMarrowState : public MarrowState {
public:
	ClawAttackMarrowState() : MarrowState(StateType::ClawAttackMarrow) {}

	void MarrowLogic(Marrow* marrow, uint32_t elapsedTime) override;
	void ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) override;
	void FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) override;
};

class ParrotTakeMarrowState : public MarrowState {
public:
	ParrotTakeMarrowState() : MarrowState(StateType::ParrotTakeMarrow) {}

	void MarrowLogic(Marrow* marrow, uint32_t elapsedTime) override;
	void ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) override;
	void FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) override;
};
