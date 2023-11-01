#pragma once

#include "BasePlaneObject.h"


class BaseCharacter : public BaseDynamicPlaneObject
{
public:
	BaseCharacter(const WwdObject& obj);

	virtual void Logic(uint32_t elapsedTime) override = 0;
	virtual void Draw() override;
	virtual Rectangle2D GetRect() override = 0;
	virtual pair<Rectangle2D, int> GetAttackRect() = 0; // returns the attack rectangle and the damage it cause
	virtual bool checkForHurts() = 0; // check if the character get hurt by player/enemy attack. returns `true` if the character get hurt

	virtual bool isStanding() const;
	virtual bool isDuck() const;
	virtual bool isTakeDamage() const;
	virtual bool isAttack() const;


protected:
	Rectangle2D setRectByCenter(Rectangle2D rc) const;
	Rectangle2D setRectByCenter(Rectangle2D rc, Rectangle2D rcCenter) const;

	map<string, shared_ptr<Animation>> _animations; // map of all animations of this character. format: [name]=animation
	Rectangle2D _lastAttackRect; // the player/enemy attackRect (that this character get, e.g. officer use it after player hit him)
	Rectangle2D _saveCurrRect;
	int _health;
	bool _isAttack;
};
