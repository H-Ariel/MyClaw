#pragma once

#include "BaseCharacter.h"
#include "Objects/Item.h"
#include "Objects/ClawProjectile.h"


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
		Invisibility, // ghost (enemies can't see him)
		Invincibility // colorful (enemies can't hurt him)
	};


	Player(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void stopFalling(float collisionSize);
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void jump(float force); // the force go to `_speed.y`

	bool collectItem(Item* item); // returns true if the item collected and should be removed. else - false

	bool isJumping() const { return speed.y < 0 && !_isOnLadder; }
	bool isClimbing() const { return _isOnLadder; }
	bool isSpikeDeath() const { return _aniName == "SPIKEDEATH"; }
	bool isFallDeath() const { return _aniName == "FALLDEATH"; }
	bool isInDeathAnimation() const { return endsWith(_aniName, "DEATH"); }
	bool isFinishDeathAnimation() const { return isInDeathAnimation() && _ani->isFinishAnimation(); }
	bool isFinishLevel() const { return _finishLevel; }
	bool isFreeze() const { return _freezeTime > 0; }
	bool isSqueezed() const { return _aniName == "SQUEEZED"; }
	bool isGhost() const { return _currPowerup == PowerupType::Invisibility; }
	bool isFalling() const override;
	bool isStanding() const override;
	bool isDuck() const override;
	bool isTakeDamage() const override;

	void backToLife();
	bool hasLives() const { return _lives > 0; }
	void loseLife();
	void setLadderFlags(bool isOnLadderTop) { _isOnLadderTop = isOnLadderTop; _isCollideWithLadder = true; }

	ClawProjectile::Types getCurrentWeapon() const { return _currWeapon; }
	int getHealthAmount() const { return _health; }
	int getLivesAmount() const { return _lives; }
	int getWeaponAmount() const { return _weaponsAmount.at(_currWeapon); }
	uint32_t getScore() const { return _score; }
	int32_t getPowerupLeftTime() const { return _powerupLeftTime; } // in milliseconds
	map<Item::Type, uint32_t> getCollectedTreasures() const { return _collectedTreasures; }

	// used to move player
	void keyUp(int key);
	void keyDown(int key);

	void activateDialog(int32_t duration) { _dialogLeftTime = duration; }

	void squeeze(D2D1_POINT_2F pos = {});
	void unsqueeze();


	D2D1_POINT_2F startPosition;
	Elevator* elevator; // stores the elevator he is standing on (if any)
	Rope* rope; // stores the rope he is holding (if any)
	

private:
	void jump();
	bool checkForHurts(); // check for hits from enemies, projectiles, and exploding powder kegs
	bool isWeaponAnimation() const;
	void calcRect(); // calculate the player rectangle and save in `_saveCurrRect`
	void calcAttackRect(); // calculate the player attack rectangle and save in `_saveCurrAttackRect`
	void shootSwordProjectile();

	string _aniName;
	vector<string> AttackAnimations, NoLoopAnimations;
	map<Item::Type, uint32_t> _collectedTreasures; // save all collected treasures and their amount
	map<size_t, PowerupSparkle> _powerupSparkles;
	map<ClawProjectile::Types, int> _weaponsAmount;
	pair<Rectangle2D, int> _saveCurrAttackRect;
	PowderKeg* _lastPowderKegExplos; // saves the last explos so he does not take damage over and over again.
	PowderKeg* _raisedPowderKeg; // saves the keg he's picking up now.
	uint32_t _score;
	int32_t _dialogLeftTime, _powerupLeftTime; // in milliseconds
	int32_t _holdAltTime; // in milliseconds. it used for pre-dynamite
	int32_t _damageRest; // rest time between enemies attacks
	int32_t _freezeTime; // in milliseconds. it used for freeze from siren.
	int _lives;
	ClawProjectile::Types _currWeapon;
	PowerupType _currPowerup;
	bool _upPressed, _downPressed, _leftPressed, _rightPressed, _spacePressed, _altPressed, _zPressed;
	bool _leftCollision, _rightCollision, _isOnLadder, _useWeapon;
	bool _finishLevel, _isCollideWithLadder, _isOnLadderTop;
};
