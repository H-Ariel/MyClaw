#pragma once

#include "BasePlaneObject.h"


class BaseCharacter : public BaseDynamicPlaneObject
{
public:
	BaseCharacter(const WwdObject& obj, Player* player = nullptr);

	virtual void Logic(uint32_t elapsedTime) override = 0;
	virtual void Draw() override;
	virtual D2D1_RECT_F GetRect() override = 0;
	// TODO: change to int16_t ?
	virtual pair<D2D1_RECT_F, int8_t> GetAttackRect() = 0; // returns the attack rectangle and the damage it cause

	virtual bool isStanding() const;
	virtual bool isDuck() const;
	virtual bool isTakeDamage() const;

	bool hasHealth() const { return _health > 0; }
	bool isAttack() const { return _isAttack; }


protected:
	map<string, shared_ptr<Animation>> _animations; // map of all animations of this character. format: [name]=animation
	D2D1_RECT_F _lastAttackRect; // the player/enemy attackRect (that this character get, e.g. officer use it after player hit him)
	D2D1_RECT_F _saveCurrRect;
	int16_t _health;
	bool _forward; // TODO use `_isMirrored`
	bool _isAttack;
};
