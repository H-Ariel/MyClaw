#pragma once

#include "../Item.h"
#include "../Projectiles/ClawProjectile.h"


// Manage life and weapons
class Inventory
{
public:
	Inventory();

	void setWeapon(ClawProjectile::Types weaponType);
	void setNextWeapon();
	ClawProjectile::Types getCurrentWeaponType() const { return _currWeapon; }
	int getCurrentWeaponAmount() const { return _weaponsAmount[(int)_currWeapon]; }
	int getWeaponAmount(ClawProjectile::Types w) const;
	void setWeaponAmount(ClawProjectile::Types w, int amount);
	void setWeaponMaxAmount(ClawProjectile::Types w);
	bool addWeapon(ClawProjectile::Types w, int amount); // add weapon if not full and returns `true`
	void useWeapon() { _weaponsAmount[(int)_currWeapon] -= 1; }
	bool hasDynamiteEquipped() const { return _currWeapon == ClawProjectile::Types::Dynamite; }

	int getLives() const { return _lives; }
	void setLives(int l) { _lives = l; }
	bool addLives(int amount); // add lives if not full and returns `true`
	void loseLife() { _lives -= 1; }
	void setMaxLives();

	map<Item::Type, uint32_t> getCollectedTreasures() const { return _collectedTreasures; }
	void collectTreasure(Item::Type treasure);
	void resetTreasures() { _collectedTreasures.clear(); }

	uint32_t getScore() const { return _score; }
	void setScore(uint32_t score) { _score = score; }

private:
	map<Item::Type, uint32_t> _collectedTreasures; // save all collected treasures and their amount
	uint32_t _score;
	int _lives;
	int _weaponsAmount[3]; // 0- pistol, 1- magic, 2- dynamite
	ClawProjectile::Types _currWeapon;
};
