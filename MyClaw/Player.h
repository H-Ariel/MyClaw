#pragma once

#include "BaseCharacter.h"
#include "Objects/Item.h"
#include "Objects/Projectile.h"


class Elevator;
class PowderKeg;
class Rope;
class PowerupSparkle;


class Player : public BaseCharacter // The legendary Captain Claw !
{
public:
	enum class PowerupType : uint8_t // TODO: use Item::Type ?
	{
		None,
		Catnip,
		FireSword,
		IceSword,
		LightningSword,
		// TODO: [fire,ice,ligh]sword, etc.
	};


	Player(const WwdObject& obj, const D2D1_SIZE_F& planeSize);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;

	bool collectItem(Item* item); // returns true if the item collected and should be removed. else - false

	bool isJumping() const;
	bool isFalling() const;
	bool isClimbing() const;
	bool isStanding() const override;
	bool isDuck() const override;
	bool isTakeDamage() const override;

	void backToLife();
	bool hasLives() const;
	void loseLife();
	bool isInDeathAnimation() const;
	bool isFinishDeathAnimation() const;

	bool isFinishLevel() const;

	ClawProjectile::Types getCurrentWeapon() const;
	int8_t getHealthAmount() const;
	int8_t getLivesAmount() const;
	int8_t getWeaponAmount() const;
	uint32_t getScore() const;
	int32_t getPowerupLeftTime() const; // in milliseconds

	// used to move player
	void keyUp(int key);
	void keyDown(int key);

	void activateDialog(int32_t duration);


	D2D1_POINT_2F startPosition;
	Elevator* elevator; // stores the elevator it is standing on (if any)
	Rope* rope;
	bool isCollideWithLadder;


private:
	void jump();
	bool checkForHurts(); // check for hits from enemies, projectiles, and exploding powder kegs
	bool isWeaponAnimation() const;


	string _aniName;
	vector<string> AttackAnimations, NoLoopAnimations;
	map<Item::Type, uint32_t> _collectedTreasures; // save all collected treasures and their amount
	map<size_t, PowerupSparkle> _powerupSparkles;
	map<ClawProjectile::Types, int8_t> _weaponsAmount;
	const D2D1_SIZE_F& _planeSize; // in pixels
	D2D1_RECT_F _saveCurrRect;
	PowderKeg* _lastPowderKegExplos; // saves the last explos so he does not take damage over and over again.
	PowderKeg* _raisedPowderKeg; // saves the keg he's picking up now.
	uint32_t _score;
	int32_t _dialogLeftTime, _powerupLeftTime; // in milliseconds
	int32_t _holdAltTime; // in milliseconds. it used for pre-dynamite
	ClawProjectile::Types _currWeapon;
	PowerupType _currPowerup;
	int8_t _lives;
	bool _upPressed, _downPressed, _leftPressed, _rightPressed, _spacePressed, _altPressed, _zPressed;
	bool _leftCollision, _rightCollision, _isOnLadder, _useWeapon;
	bool _finishLevel;
};
