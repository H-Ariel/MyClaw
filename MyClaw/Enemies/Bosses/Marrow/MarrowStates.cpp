#include "MarrowStates.h"
#include "../../../GlobalObjects.h"


#define MARROW_ANIMATION_HAND_UP		marrow->_animations["IDLE1"] // raise his hand up
#define MARROW_ANIMATION_WAIT_HAND_UP	marrow->_animations["IDLE2"] // wait with hand up (eyes open)
#define MARROW_ANIMATION_HAND_DOWN		marrow->_animations["IDLE4"] // get hand down back
#define MARROW_ANIMATION_HOME			marrow->_animations["HOME"] // just stand


void FirstFloorsRemoveState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {}
void FirstFloorsRemoveState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {}
void FirstFloorsRemoveState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {
	floor->moveFloor(elapsedTime, false);
}

void ParrotAttackClawState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {}
void ParrotAttackClawState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {
	bool speedChanged = true;

	if (parrot->position.y > parrot->_flyRect.bottom)
	{
		parrot->position.y = parrot->_flyRect.bottom;
		parrot->speed = { -MarrowParrot::PARROT_SPEED, 0 };
	}
	else if (parrot->position.y < parrot->_flyRect.top)
	{
		parrot->position.y = parrot->_flyRect.top;
		parrot->speed = { MarrowParrot::PARROT_SPEED, 0 };
	}
	else if (parrot->position.x < parrot->_flyRect.left)
	{
		parrot->position.x = parrot->_flyRect.left;
		parrot->speed = { 0, -MarrowParrot::PARROT_SPEED };
	}
	else if (parrot->position.x > parrot->_flyRect.right)
	{
		parrot->position.x = parrot->_flyRect.right;
		parrot->speed = { 0, MarrowParrot::PARROT_SPEED };
	}
	else if (GO::isPlayerTakeDamage()) // if player is hurt, parrot returns to Marrow
	{
		parrot->speed = { 0, -MarrowParrot::PARROT_SPEED };
	}
	else speedChanged = false;

	if (speedChanged && Marrow::getInstance()->isInLefttSide())
	{
		parrot->speed.x = -parrot->speed.x;
		parrot->speed.y = -parrot->speed.y;
	}

	if (parrot->_hitsCounter == 3)
	{
		parrot->_hitsCounter = 0;
		Marrow::getInstance()->switchState(DBG_NEW AddFloorState());
	}
}
void ParrotAttackClawState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {}

void AddFloorState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {}
void AddFloorState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {}
void AddFloorState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {
	floor->moveFloor(elapsedTime, true);
}

void ClawAttackMarrowState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {
	if (marrow->_hitsCuonter == 5)
	{
		marrow->_hitsCuonter = 0;
		marrow->switchState(DBG_NEW ParrotTakeMarrowState());
		marrow->_ani = MARROW_ANIMATION_HAND_UP;
		marrow->_ani->reset();
		marrow->_ani->loopAni = false;
	}
}
void ClawAttackMarrowState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {
	parrot->position = parrot->_initialPosition;
}
void ClawAttackMarrowState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {}

void ParrotTakeMarrowState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {
	if (marrow->_ani == MARROW_ANIMATION_HAND_UP && marrow->_ani->isFinishAnimation())
	{
		marrow->_isMirrored = marrow->_side;
		marrow->speed.x = marrow->_isMirrored ? -0.4f : 0.4f;
		marrow->position.y -= 128;
		marrow->_ani = MARROW_ANIMATION_WAIT_HAND_UP;
		marrow->_ani->reset();
		marrow->_ani->loopAni = false;
	}
	else if (marrow->_ani == MARROW_ANIMATION_WAIT_HAND_UP && marrow->speed.x == 0) // Marrow stops move
	{
		marrow->position.y += 128;
		marrow->_isMirrored = marrow->_side;
		marrow->_side = !marrow->_side;
		marrow->switchState(DBG_NEW ParrotAttackClawState());
		marrow->_ani = MARROW_ANIMATION_HOME;
		marrow->_ani->reset();
	}

	marrow->position.x += marrow->speed.x * elapsedTime;
}
void ParrotTakeMarrowState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {
	parrot->_ani = parrot->_animations["FLY"];
	parrot->_isAttack = false;
	parrot->_isMirrored = Marrow::getInstance()->isMirrored();
	parrot->position = Marrow::getInstance()->position;
	parrot->position.y -= TILE_SIZE;
	if (parrot->_isMirrored) parrot->position.x -= 0.75f * TILE_SIZE;
	else parrot->position.x += 0.75f * TILE_SIZE;
}
void ParrotTakeMarrowState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {
	floor->moveFloor(elapsedTime, false); // when parrot takes Marrow, floors remove
}
