#pragma once

#include "Player.h"


// Level Head-Up Display
class LevelHUD : public UIBaseElement
{
public:
	LevelHUD(const D2D1_POINT_2F& offset);
	~LevelHUD();

	void Draw() override;

	static void setBossInitialHealth(int health) { _bossInitialHealth = health; }

private:
	void drawNumbers(uint32_t amount, int numOfDigits, shared_ptr<UIBaseImage> const numArr[], D2D1_POINT_2F pos, bool isScore = false) const;

	shared_ptr<Animation> _chest, _health, _lives, _stopwatch, _bossBar;
	map<ClawProjectile::Types, shared_ptr<Animation>> _weaponAni;
	shared_ptr<UIBaseImage> _healthNumbers[10], _smallNumbers[10], _scoreNumbers[10];
	const D2D1_POINT_2F& _offset;
	
	static int _bossInitialHealth;
};
