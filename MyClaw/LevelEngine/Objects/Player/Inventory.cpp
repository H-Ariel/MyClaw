#include "Inventory.h"


constexpr int MAX_WEAPON_AMOUNT = 99;
constexpr int MAX_LIVES_AMOUNT = 9;


#define CHECK_INVALID_AND_RETURN(w) if (w != ClawProjectile::Types::Pistol && \
	w != ClawProjectile::Types::Magic && w != ClawProjectile::Types::Dynamite) \
		return // this is an open `return` statement, see why in the code :)


Inventory::Inventory()
	: _score(0), _lives(6), _currWeapon(ClawProjectile::Types::Pistol)
{
	_weaponsAmount[(int)ClawProjectile::Types::Pistol] = 10;
	_weaponsAmount[(int)ClawProjectile::Types::Magic] = 5;
	_weaponsAmount[(int)ClawProjectile::Types::Dynamite] = 3;
}

void Inventory::setWeapon(ClawProjectile::Types w) {
	CHECK_INVALID_AND_RETURN(w);
	_currWeapon = w;
}

void Inventory::setNextWeapon() {
	_currWeapon = ClawProjectile::Types(((int)_currWeapon + 1) % 3); // 3=_ARRAYSIZE(_weaponsAmount)
}

int Inventory::getWeaponAmount(ClawProjectile::Types w) const {
	CHECK_INVALID_AND_RETURN(w) 0;
	return _weaponsAmount[(int)w];
}
void Inventory::setWeaponAmount(ClawProjectile::Types w, int amount) {
	CHECK_INVALID_AND_RETURN(w);

	_weaponsAmount[(int)w] = amount;
	if (_weaponsAmount[(int)w] > MAX_WEAPON_AMOUNT)
		_weaponsAmount[(int)w] = MAX_WEAPON_AMOUNT;
}
void Inventory::setWeaponMaxAmount(ClawProjectile::Types w) {
	CHECK_INVALID_AND_RETURN(w);

	_weaponsAmount[(int)w] = MAX_WEAPON_AMOUNT;
}

bool Inventory::addWeapon(ClawProjectile::Types w, int amount) {
	CHECK_INVALID_AND_RETURN(w) false;

	if (_weaponsAmount[(int)w] < MAX_WEAPON_AMOUNT) {
		setWeaponAmount(w, _weaponsAmount[(int)w] + amount);
		return true;
	}

	return false;
}


bool Inventory::addLives(int amount) {
	if (_lives < MAX_LIVES_AMOUNT) {
		_lives += amount;
		if (_lives > MAX_LIVES_AMOUNT)
			_lives = MAX_LIVES_AMOUNT;
		return true;
	}

	return false;
}
void Inventory::setMaxLives()
{
	_lives = MAX_LIVES_AMOUNT;
}


void Inventory::collectTreasure(Item::Type treasure) { 
	_collectedTreasures[treasure] += 1; 

	// each 500,000 points CC gets an extra life
	// NOTE we divide `int`s, so possible results are [0,1,2,...]
	uint32_t lastScoreForExtraLife = _score / 500000;
	_score += Item::getTreasureScore(treasure);
	if (lastScoreForExtraLife < _score / 500000)
		addLives(1);
}
