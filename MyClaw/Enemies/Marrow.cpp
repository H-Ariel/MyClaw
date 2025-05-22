#include "Marrow.h"
#include "../GlobalObjects.h"
#include "../Objects/ClawProjectile.h"


#define MARROW_ANIMATION_BLOCK			_animations["BLOCK"]
#define MARROW_ANIMATION_HOME			_animations["HOME"] // just stand
#define MARROW_ANIMATION_HAND_UP		_animations["IDLE1"] // raise his hand up
#define MARROW_ANIMATION_WAIT_HAND_UP	_animations["IDLE2"] // wait with hand up (eyes open)
#define MARROW_ANIMATION_HAND_DOWN		_animations["IDLE4"] // get hand down back

#define PARROT_ANIMATION_STRIKE		_animations["STRIKE1"]

constexpr float PARROT_SPEED = 0.4f;
constexpr int MAX_FLOORS_COUNT = 2;


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


static Marrow* _Marrow = nullptr; // TODO make this no static... maybe class field

int8_t MarrowFloor::floorCounter = 0;


Marrow::Marrow(const WwdObject& obj)
	: BaseBoss(obj, "KILLFALL"), _side(Side::Right)
{
	speed.x = 0;
	_health = 100;
	_ani = MARROW_ANIMATION_HOME;
	_isMirrored = true;
	_Marrow = this;
	_state = DBG_NEW FirstFloorsRemoveState();
}
Marrow::~Marrow() { delete _state; _Marrow = nullptr; }
void Marrow::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;
	if (_ani == MARROW_ANIMATION_BLOCK)
	{
		if (_ani->isFinishAnimation())
		{
			_ani = MARROW_ANIMATION_HOME;
		}
		else
		{
			PostLogic(elapsedTime);
			return;
		}
	}

	_blockClaw = (_hitsCuonter == 1 && !_blockClaw);
	_state->MarrowLogic(this, elapsedTime);

	if (_isAttack)
	{
		if (_ani->isFinishAnimation())
		{
			_ani = MARROW_ANIMATION_HOME;
			_ani->reset();
			_isAttack = false;
		}
	}
	else
	{
		if (_state->getType() == MarrowState::StateType::ClawAttackMarrow)
			BaseBoss::makeAttack();
	}

	PostLogic(elapsedTime);
}
void Marrow::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 96 && deltaY < 16)
	{
		_ani = _animations["STRIKE2"]; // knife attack
		_ani->reset();
		_isAttack = true;
		_isMirrored = GO::getPlayerPosition().x < position.x;
	}
}
bool Marrow::checkForHurts()
{
	if (_state->getType() == MarrowState::StateType::ClawAttackMarrow)
	{
		for (Projectile* p : GO::getActionPlaneProjectiles())
			if (isinstance<ClawProjectile>(p))
				if (_saveCurrRect.intersects(p->GetRect()))
				{
					_ani = MARROW_ANIMATION_BLOCK;
					_ani->reset();
					return false;
				}

		if (checkClawHit())
			if (_blockClaw)
			{
				_ani = MARROW_ANIMATION_BLOCK;
				return false;
			}
			else
			{
				if (isinstance<ClawAttackMarrowState>(_state))
					_hitsCuonter += 1;
				return true;
			}
	}

	return false;
}
void Marrow::stopMovingLeft(float collisionSize)
{
	BaseBoss::stopMovingLeft(collisionSize);
	speed = {};
	_isMirrored = true;
}
void Marrow::stopMovingRight(float collisionSize)
{
	BaseBoss::stopMovingRight(collisionSize);
	speed = {};
	_isMirrored = false;
}
pair<Rectangle2D, int> Marrow::GetAttackRect() { return {}; }
void Marrow::ClawAttackMarrowStateLogic(uint32_t elapsedTime) {
	if (_hitsCuonter == 5)
	{
		_hitsCuonter = 0;
		switchState(DBG_NEW ParrotTakeMarrowState());
		_ani = MARROW_ANIMATION_HAND_UP;
		_ani->reset();
		_ani->loopAni = false;
	}
}
void Marrow::ParrotTakeMarrowStateLogic(uint32_t elapsedTime) {
	if (_ani == MARROW_ANIMATION_HAND_UP && _ani->isFinishAnimation())
	{
		_isMirrored = _side == Marrow::Side::Right;
		speed.x = _isMirrored ? -0.4f : 0.4f;
		position.y -= 128;
		_ani = MARROW_ANIMATION_WAIT_HAND_UP;
		_ani->reset();
		_ani->loopAni = false;
	}
	else if (_ani == MARROW_ANIMATION_WAIT_HAND_UP && speed.x == 0) // Marrow stops move
	{
		position.y += 128;
		_isMirrored = _side == Marrow::Side::Right;
		_side = _isMirrored ? Marrow::Side::Left : Marrow::Side::Right;
		switchState(DBG_NEW ParrotAttackClawState());
		_ani = MARROW_ANIMATION_HOME;
		_ani->reset();
	}

	position.x += speed.x * elapsedTime;

}
void Marrow::switchState(MarrowState* newState) {
	delete _state;
	_state = newState;
}


MarrowParrot::MarrowParrot(const WwdObject& obj)
	: BaseEnemy(obj, 3, 10, "FLY", "HIT", "HIT", "KILLFALL", "STRIKE1", "",
		"", "", "", PARROT_SPEED, true), _hitsCounter(0), _initialPosition({}),
	_flyRect((float)obj.minX, (float)obj.minY - 32.f, (float)obj.maxX, (float)obj.maxY - 32.f)
{
	myMemCpy(_initialPosition, position);
	_isMirrored = true;
	speed = { 0, PARROT_SPEED };
}
void MarrowParrot::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_Marrow) {
		position.x += speed.x * elapsedTime;
		position.y += speed.y * elapsedTime;

		_Marrow->getState()->ParrotLogic(this, elapsedTime);

		if ((speed.x > 0 && _Marrow->getSide() == Marrow::Side::Left) ||
			(speed.x < 0 && _Marrow->getSide() == Marrow::Side::Right))
		{
			_ani = PARROT_ANIMATION_STRIKE;
			_isAttack = true;
		}
		else
		{
			_ani = _animations["FLY"];
			_isAttack = false;
		}

		if (_Marrow->getState()->getType() != MarrowState::StateType::ParrotTakeMarrow)
			_isMirrored = speed.x < 0;
	}

	PostLogic(elapsedTime);
}
pair<Rectangle2D, int> MarrowParrot::GetAttackRect() { return { GetRect(), _damage }; }
bool MarrowParrot::checkForHurts()
{
	int health = _health; // save health value

	if (_isAttack && checkForHurt(GO::getPlayerAttackRect()))
	{
		_hitsCounter += 1;
		speed = { 0, -PARROT_SPEED };
		_ani = _animations["HIT"];
		_ani->reset();
	}

	// make sure value did not changed (parrot could not die)
	_health = health;
	removeObject = false;

	return false;
}
// the Parrot is flying so if it touch ground we ignore it
void MarrowParrot::stopFalling(float collisionSize) {}
void MarrowParrot::stopMovingLeft(float collisionSize) {}
void MarrowParrot::stopMovingRight(float collisionSize) {}
void MarrowParrot::bounceTop() {}
void MarrowParrot::ParrotAttackClawStateLogic(uint32_t elapsedTime) {
	bool speedChanged = true;

	if (position.y > _flyRect.bottom)
	{
		position.y = _flyRect.bottom;
		speed = { -PARROT_SPEED, 0 };
	}
	else if (position.y < _flyRect.top)
	{
		position.y = _flyRect.top;
		speed = { PARROT_SPEED, 0 };
	}
	else if (position.x < _flyRect.left)
	{
		position.x = _flyRect.left;
		speed = { 0, -PARROT_SPEED };
	}
	else if (position.x > _flyRect.right)
	{
		position.x = _flyRect.right;
		speed = { 0, PARROT_SPEED };
	}
	else if (GO::isPlayerTakeDamage()) // if player is hurt, parrot returns to Marrow
	{
		speed = { 0, -PARROT_SPEED };
	}
	else speedChanged = false;

	if (speedChanged && _Marrow->getSide() == Marrow::Side::Left)
	{
		speed.x = -speed.x;
		speed.y = -speed.y;
	}

	if (_hitsCounter == 3)
	{
		_hitsCounter = 0;
		_Marrow->switchState(DBG_NEW AddFloorState());
	}
}
void MarrowParrot::ClawAttackMarrowStateLogic(uint32_t elapsedTime) {
	position = _initialPosition;
}
void MarrowParrot::ParrotTakeMarrowStateLogic(uint32_t elapsedTime) {
	_ani = _animations["FLY"];
	_isAttack = false;
	_isMirrored = _Marrow->isMirrored();
	position = _Marrow->position;
	position.y -= TILE_SIZE;
	if (_isMirrored) position.x -= 0.75f * TILE_SIZE;
	else position.x += 0.75f * TILE_SIZE;
}


MarrowFloor::MarrowFloor(const WwdObject& obj)
	: BaseStaticPlaneObject(obj),
	_minX((float)obj.minX), _maxX((float)obj.maxX),
	_speedX((obj.direction ? obj.speedX : -obj.speedX) / 1000.f)
{
	_ani = AssetsManager::createAnimationFromPidImage("LEVEL10/IMAGES/TRAPELEVATOR/001.PID");
}
void MarrowFloor::Logic(uint32_t elapsedTime)
{
	tryCatchPlayer();
	if (_Marrow)
		_Marrow->getState()->FloorLogic(this, elapsedTime);
}
Rectangle2D MarrowFloor::GetRect()
{
	return BasePlaneObject::GetRect();
}
void MarrowFloor::removeFloor(uint32_t elapsedTime) {
	position.x += _speedX * elapsedTime;
	if (position.x < _minX)
	{
		position.x = _minX;
		floorCounter += 1;
	}
	else if (position.x > _maxX)
	{
		position.x = _maxX;
		floorCounter += 1;
	}

	if (floorCounter == MAX_FLOORS_COUNT)
	{
		floorCounter = 0;
		if (_Marrow->getState()->getType() == MarrowState::StateType::FirstFloorsRemove)
			_Marrow->switchState(DBG_NEW ParrotAttackClawState());
	}
}
void MarrowFloor::AddFloorStateLogic(uint32_t elapsedTime)
{
	position.x -= _speedX * elapsedTime;
	if (position.x < _minX)
	{
		position.x = _minX;
		floorCounter += 1;
	}
	else if (position.x > _maxX)
	{
		position.x = _maxX;
		floorCounter += 1;
	}

	if (floorCounter == MAX_FLOORS_COUNT)
	{
		// all floors finished their job
		floorCounter = 0;
		_Marrow->switchState(DBG_NEW ClawAttackMarrowState());
	}
}
void MarrowFloor::ParrotTakeMarrowStateLogic(uint32_t elapsedTime) {
	removeFloor(elapsedTime); // when parrot take Marrow, floors remove
}
void MarrowFloor::FirstFloorsRemoveStateLogic(uint32_t elapsedTime) {
	removeFloor(elapsedTime);
}


void FirstFloorsRemoveState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {}
void FirstFloorsRemoveState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {}
void FirstFloorsRemoveState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {
	floor->FirstFloorsRemoveStateLogic(elapsedTime);
}

void ParrotAttackClawState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {}
void ParrotAttackClawState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {
	parrot->ParrotAttackClawStateLogic(elapsedTime);
}
void ParrotAttackClawState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {}

void AddFloorState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {}
void AddFloorState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {}
void AddFloorState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {
	floor->AddFloorStateLogic(elapsedTime);
}

void ClawAttackMarrowState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {
	marrow->ClawAttackMarrowStateLogic(elapsedTime);
}
void ClawAttackMarrowState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {
	parrot->ClawAttackMarrowStateLogic(elapsedTime);
}
void ClawAttackMarrowState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {}

void ParrotTakeMarrowState::MarrowLogic(Marrow* marrow, uint32_t elapsedTime) {
	marrow->ParrotTakeMarrowStateLogic(elapsedTime);
}
void ParrotTakeMarrowState::ParrotLogic(MarrowParrot* parrot, uint32_t elapsedTime) {
	parrot->ParrotTakeMarrowStateLogic(elapsedTime);
}
void ParrotTakeMarrowState::FloorLogic(MarrowFloor* floor, uint32_t elapsedTime) {
	floor->ParrotTakeMarrowStateLogic(elapsedTime);
}
