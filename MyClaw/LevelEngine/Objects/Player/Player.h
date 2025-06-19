#pragma once

#include "../BaseCharacter.h"
#include "SavedDataManager.h"
#include "../Projectiles/ClawProjectile.h"
#include "Inventory.h"
#include "PowerupSparkles.h"


class Elevator;
class PowderKeg;
class Rope;
class ConveyorBelt;
class PowerupSparkle;


class Player : public BaseCharacter // The legendary Captain Claw !
{
public:
	Player();
	~Player();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void whenTouchDeath() override;
	void jump(float force); // the force go to `_speed.y`

	bool collectItem(Item* item); // returns `true` if the item collected and should be removed. else - `false`.

	bool isJumping() const { return speed.y < 0 && !_isOnLadder; }
	bool isClimbing() const { return _isOnLadder; }
	bool isSpikeDeath() const { return _aniName == "SPIKEDEATH"; }
	bool isFallDeath() const { return _aniName == "FALLDEATH"; }
	bool isInDeathAnimation() const { return endsWith(_aniName, "DEATH"); }
	bool isFinishDeathAnimation() const { return isInDeathAnimation() && _ani->isFinishAnimation(); }
	bool isFinishLevel() const { return _finishLevel; }
	bool isFreeze() const { return _freezeTime > 0; }
	bool isSqueezed() const { return _aniName == "SQUEEZED"; }
	bool isGhost() const { return _currPowerup == Item::Powerup_Invisibility; }
	bool isInvincibility() const { return _currPowerup == Item::Powerup_Invincibility; }
	bool isFalling() const override;
	bool isStanding() const override;
	bool isDuck() const override;
	bool isTakeDamage() const override;

	void backToLife();
	bool hasLives() const { return _inventory.getLives() > 0; }
	void loseLife();
	void setLadderFlags(bool isOnLadderTop) { _isOnLadderTop = isOnLadderTop; _isCollideWithLadder = true; }
	void nextLevel();
	void endLife(); // at ingame-menu

	ClawProjectile::Types getCurrentWeaponType() const { return _inventory.getCurrentWeaponType(); }
	int getCurrentWeaponAmount() const { return _inventory.getCurrentWeaponAmount(); }
	int getLivesAmount() const { return _inventory.getLives(); }
	uint32_t getScore() const { return _inventory.getScore(); }
	int getPowerupLeftTime() const { return _powerupLeftTime; } // in milliseconds
	map<Item::Type, uint32_t> getCollectedTreasures() const { return _inventory.getCollectedTreasures(); }

	SavedDataManager::GameData getGameData() const;
	void setGameData(const SavedDataManager::GameData& data);

	void activateDialog(int duration) { _dialogLeftTime = duration; }

	void squeeze(D2D1_POINT_2F pos, bool mirror = false);
	void unsqueeze();

	bool cheat(int cheatType); // return `true` if the cheat is activated


	D2D1_POINT_2F startPosition;
	Elevator* elevator; // stores the elevator he is standing on (if any)
	Rope* rope; // stores the rope he is holding (if any)
	ConveyorBelt* conveyorBelt; // stores the belt he is standing on (if any)


private:
	void useWeapon(bool duck, bool inAir);
	void jump();
	bool checkForHurts() override; // check for hits from enemies, projectiles, and exploding powder kegs
	bool isWeaponAnimation() const;
	void calcRect(); // calculate the player rectangle and save in `_saveCurrRect`
	void calcAttackRect(); // calculate the player attack rectangle and save in `_saveCurrAttackRect`
	void shootSwordProjectile();

	string _aniName;
	vector<string> AttackAnimations, NoLoopAnimations;
	PowerupSparkles _powerupSparkles;
	pair<Rectangle2D, int> _saveCurrAttackRect;
	Inventory _inventory;
	PowderKeg* _raisedPowderKeg; // saves the keg he's picking up now.
	int _dialogLeftTime, _powerupLeftTime; // in milliseconds
	int _holdAltTime; // in milliseconds. it used for pre-dynamite
	int _damageRest; // rest time between enemies attacks
	int _freezeTime; // in milliseconds. it used for freeze from siren.
	Item::Type _currPowerup; // the current powerup he has (not treasures!)
	bool _upPressed, _downPressed, _leftPressed, _rightPressed, _spacePressed, _altPressed, _zPressed;
	bool _leftCollision, _rightCollision, _isOnLadder, _useWeapon;
	bool _finishLevel, _isCollideWithLadder, _isOnLadderTop;


	friend class InputController;
};
