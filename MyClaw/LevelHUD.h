#pragma once

#include "Player.h"


// Level Head-Up Display
class LevelHUD : public UIBaseElement
{
public:
	LevelHUD(const D2D1_POINT_2F* windowOffset);

	void Draw() override;


private:
	void drawNumbers(uint32_t amount, int numOfDigits, shared_ptr<UIBaseImage> const numArr[], float posX, float posY) const;

	shared_ptr<UIAnimation> _chest, _health, _lives, _stopwatch;
	shared_ptr<UIBaseImage> _bossBar;
	map<ClawProjectile::Types, shared_ptr<UIAnimation>> _weaponAni;
	shared_ptr<UIBaseImage> _healthNumbers[10], _smallNumbers[10], _scoreNumbers[10];
	const D2D1_POINT_2F* _windowOffset; // save the offset because WindowManager uses it in the drawing and HUD uses a different offset
	int _bossHealth;
};
